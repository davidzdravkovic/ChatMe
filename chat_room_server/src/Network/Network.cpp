#include "../include/Network/Network.h"
#include "../include/Network/RequestPipeline.h"
#include "../include/Deserialization/DTO/RequestStruct.h"
#include <boost/system.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using json = nlohmann::json;

namespace {

bool isClientActive(const std::shared_ptr<Client>& c) {
    return c && c->getState().load() == ClientState::Active;
}

} // namespace

std::shared_ptr<Client> Network::client(int sessionId) {
    return activeConnects.getClient(sessionId);
}

void Network::processRequest(int sessionId, std::string jsonStr) {
   std::cout << "[DEBUG] Received JSON: " << jsonStr << std::endl;

    ingress_pipeline::RequestContext ctx;
    ctx.sessionId = sessionId;
    ctx.rawJson = std::move(jsonStr);

    const auto sendIngressError = [this](int sid, const std::vector<uint8_t>& payload) {
        enqueueClientPayload(sid, payload);
        writeWs(sid);
    };
    const auto scheduleRead = [this](int sid) { readWs(sid);};
    const auto getClient = [this](int sid) { return client(sid); };

    const auto outcome = ingress_pipeline::RequestPipeline::run(ctx, getClient, sendIngressError, scheduleRead);

    switch (outcome) {
    case ingress_pipeline::RequestPipelineOutcome::Dispatch:
        traffic.route(ctx.request);
        readWs(sessionId);
        break;
    case ingress_pipeline::RequestPipelineOutcome::RescheduleRead:
        readWs(sessionId);
        break;
    case ingress_pipeline::RequestPipelineOutcome::Idle:
        break;
    }
}

// --- WebSocket: accept (mirrors former accept) ---
void Network::acceptWs() {
    wsAcceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) mutable {
            if (ec) {
                acceptWs();
                return;
            }

            auto ws = std::make_shared<WsStream>(std::move(socket));
    
            /// |1000 bytes| |
            // Idle, handshake limits, and optional keep_alive PINGs are handled inside the stream (see Beast websocket timeouts).
            auto timeOut = boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server);
            ws->set_option(timeOut);
            ws->set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& res) {
                    res.set(boost::beast::http::field::server, "C++ WebSocket Server");
                }));

            ws->async_accept( 
                [this, ws](boost::system::error_code ec) {
                    if (ec) {
                        acceptWs();
                        return;
                    }

                   
                    int sessionId = activeConnects.createClient(ws);
                    if (sessionId == 0) {
                        acceptWs();
                        return;
                    }

                    json sessionInit;
                    sessionInit["type"] = "SESSION_INIT";
                    sessionInit["sessionId"] = sessionId;
                    const std::string payload = sessionInit.dump();
                    std::vector<uint8_t> sessionData(payload.begin(), payload.end());
                    enqueueClientPayload(sessionId, sessionData);
                    writeWs(sessionId); 
                    readWs(sessionId);
                    acceptWs();
                }
            );
        }
    );
}


//WebSocket: read loop (IO only; logic in processRequest + ingress_pipeline)
void Network::readWs(int sessionId) {
    std::shared_ptr<Client> sessionClient = client(sessionId);
    if (!isClientActive(sessionClient)) {
        return;
    }
    auto buffer = std::make_shared<boost::beast::flat_buffer>();
    sessionClient->getWs().async_read(
        *buffer,
        [this, buffer, client = std::move(sessionClient), sessionId](boost::system::error_code ec, std::size_t) {
            if (ec) {
                if (ec == boost::beast::error::timeout) {
                    std::cout << "[WS] session " << sessionId << " read: idle/timeout — " << ec.message()
                              << std::endl;
                } else {
                    std::cout << "[WS] session " << sessionId << " read error: " << ec.message() << " ("
                              << ec.category().name() << " " << ec.value() << ")" << std::endl;
                }
                activeConnects.emitDisconnect(sessionId);
                return;
            }
            if (!isClientActive(client)) {
                return;
            }
      
            std::string jsonStr = boost::beast::buffers_to_string(buffer->data());
            processRequest(sessionId, std::move(jsonStr));
        });
}


// WebSocket: write (mirrors former write)
void Network::writeWs(int sessionId) {
    std::shared_ptr<Client> sessionClient = client(sessionId);
    if (!isClientActive(sessionClient)) {
        return;
    }

    //checks if for the client there is pending writing if is not in flight modifies a flag to true with CAS atomic and return true
    if (!sessionClient->notInFlight())
        return; 
        
       auto body = std::make_shared<std::vector<uint8_t>>(sessionClient->dequeueMessage());
    
    std::string raw(body->begin(), body->end());
    try {
        auto j = json::parse(raw);
        std::cout << "[DEBUG] JSON to send: " << j.dump(2) << std::endl;
    } catch (...) {
        std::cout << "Non-JSON payload (" << body->size() << " bytes)" << std::endl;
    }
    std::cout << "SEND SIZE: " << body->size() << std::endl;

    sessionClient->getWs().text(true);
    sessionClient->getWs().async_write( boost::asio::buffer(*body), [this, client = std::move(sessionClient), body](boost::system::error_code ec, std::size_t) {
            if (ec) {
                activeConnects.emitDisconnect(client->getId());
                return;
            }
            //On successful write the lock is unlocked and if any write is returned while the success write was executing, for that write is started new async write
            client->finishInFlight();
            if (client->hasMessagesToSend()) {
                writeWs(client->getId());
            } else if (client->getAuthRejectClose()) {
                activeConnects.emitDisconnect(client->getId());
            }
        });


}

 void Network::enqueueClientPayload(const int &sessionId, const std::vector<uint8_t> &payload) {
    //For sending messages we are getting the other client from active clients to queue the message
    //The client may be offline if before this function the client is offline this queue is never happening
    //If the client has queue message and later gets offline the write handles that gratefully
    std::shared_ptr<Client> sessionClient = client(sessionId);
        
    if (!sessionClient) return;
    sessionClient->enqueueMessage(payload);
}

void Network::applyActions(const std::vector<NetworkAction>& actions) {
    for (auto& a : actions) {
        switch (a.type) {
            case NetworkAction::ActionType::LOGOUT:
                break;
           
            case NetworkAction::ActionType::SEND_AUTH_RESPONSE:
                if (client(a.sessionId)) {
                    enqueueClientPayload(a.sessionId, a.payload);
                    writeWs(a.sessionId);
   
                }
            //This case for passed authentication prevents holding a user in the business stack as orphan
            //as error removes the client from the connections and also from the business stack, but the login/create request create entry after the cleaning of the business stack
            //If there is no client maybe the error path has not yet removed the client from the business stack, then is harmless two request path for same client cleaning to be executed.
            //if the path of cleaning the client is LOGOUT REQUEST the LOGIN/CREATE request are taken of as early as possible with the flag LogOutPending which if true
            //Forbids any request of any type to be executed.
                else {
                    RequestStruct disconnectReq;
                    disconnectReq.rType = RequestType::DISCONNECT_REQUEST;
                    disconnectReq.sessionID = a.sessionId;
                    disconnectReq.data = std::to_string(a.sessionId);
                    traffic.route(disconnectReq);
                }
                break;
            case NetworkAction::ActionType::SEND_TO_SESSION:
                //If session id is not bind to a client the enqueue fails
                enqueueClientPayload(a.sessionId, a.payload);
                writeWs(a.sessionId);
                break;
              //After we take a care of the ordered message to take its place in the write queue and reserves its own order of writting (FIFO)
              //We can safely release the new message from the pending ordered queue which will be also in (FIFO manner), and after business logic is applied to it
              //Eventually will take writing order AFTER already settled messages in the pending writting queue, binding 2 FIFO queues the order is established for ordered messages type
              //Which ordered message is read first is finished (written a response for it to the client first).   
            case NetworkAction::ActionType::RELEASE_ORDERED_GATE: {
                std::shared_ptr<Client> sessionClient = client(a.sessionId);
                if (sessionClient) {
                    RequestStruct next;
                    if (sessionClient->releaseOrderedRead(next))
                        traffic.route(next);
                }
                break;
            }
            case NetworkAction::ActionType::USER_OFFLINE:
                break;
            case NetworkAction::ActionType::NOOP:
                break;
            case NetworkAction::ActionType::FORCE_DISCONNECT:
                activeConnects.emitDisconnect(a.sessionId);
                break;
            default:
                break;
        }
    }
}

std::function<void(std::vector<NetworkAction>)> Network::makeDispatcher() {
    return [this](std::vector<NetworkAction> actions) {
        boost::asio::post(io, [this, actions = std::move(actions)] {
            applyActions(actions);
        });
    };
}

void Network::run() {
    acceptWs();
    // Three worker threads share the same io_context (Boost.Asio pool pattern).
    // workGuard keeps the io_context from running out of work while listeners are up.
    static constexpr std::size_t kNetworkIoThreads = 3;
    std::vector<std::thread> ioWorkers;
    ioWorkers.reserve(kNetworkIoThreads);
    for (std::size_t i = 0; i < kNetworkIoThreads; ++i) {
        ioWorkers.emplace_back([this]() { io.run(); });
    }
    for (auto& t : ioWorkers) {
        t.join();
    }
}  
