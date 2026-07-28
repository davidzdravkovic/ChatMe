#include "../include/Network/Client.h"
#include <boost/beast/core.hpp>
#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>

namespace {
std::atomic<std::uint64_t> g_asyncCloseCompletions{0};
std::atomic<std::uint64_t> g_clientDestructions{0};
std::mutex g_clientLifeLogMtx;
} // namespace

Client::Client(std::shared_ptr<WsStream> wsStream, int iD)
    : sessionID(iD), ws(std::move(wsStream)) {}

void Client::beginClose(std::shared_ptr<Client> self, std::function<void(int)> onClosed) {
    assert(self.get() == this);

    ClientState expected = ClientState::Active;
    //Dynamic state check if some async operations passed the extraction of the map check and they as handlers are going to start new async work
    //This check is not needed for async write handler because the write handler makes a new write that is going to check the map in the first moment
    //While closing is happening no new operations async are allowed
    if (!state.compare_exchange_strong(expected, ClientState::Closing)) {
        return;
    }

    boost::system::error_code ec;
    //If any async operations are pending they are cancelled
    //Pending or executing handlers are not cancelled affected and they can still create new async work but state check in the beginning for read and for write handler the map 
    //prevents to start new async work
    ws->next_layer().cancel(ec);

    // Move `self` into the completion so this Client (and the socket) stay alive until the handler runs.
    ws->async_close(
        boost::beast::websocket::close_code::normal,
        [self = std::move(self), onClosed = std::move(onClosed), this](boost::system::error_code ec) {
            (void)ec;
            g_asyncCloseCompletions.fetch_add(1, std::memory_order_relaxed);
            {
                std::lock_guard<std::mutex> lock(g_clientLifeLogMtx);
                std::cout << "[ClientLife] sessionId=" << sessionID
                          << " async_close completion totals: asyncClose=" << g_asyncCloseCompletions.load()
                          << " dtor=" << g_clientDestructions.load() << std::endl;
            }

            boost::system::error_code close_ec;
            auto& sock = boost::beast::get_lowest_layer(*ws);
            if (sock.is_open()) {
                sock.close(close_ec);
            }

            state.store(ClientState::Closed);
            if (onClosed) {
                onClosed(sessionID);
            }
        }
    );
}

void Client::enqueueMessage(const std::vector<uint8_t> message) {
    {
        std::lock_guard<std::mutex> lock(sendQueueMutex);
        messagesToSend.push(message);
    }
  
}
bool Client::hasMessagesToSend() {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    return !messagesToSend.empty();
}
void Client::finishInFlight() {
    isWriting.store(false);
}

bool Client::notInFlight() {
    bool expected = false;
    if(isWriting.compare_exchange_strong(expected,true)){
        return true;
    }
    return false;
}
std::vector<uint8_t> Client::dequeueMessage() {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    if (messagesToSend.empty()) {
        return {};
    }
    auto message = messagesToSend.front();
    messagesToSend.pop();
    return message;
}

bool Client::tryAcquireOrderedRead(const RequestStruct& req) {
    std::lock_guard<std::mutex> lock(orderedReadMtx);
    if (orderedReadInFlight) {
        pendingOrderedReads.push(req);
        return false;
    }
    orderedReadInFlight = true;
    return true;
}

bool Client::releaseOrderedRead(RequestStruct& next) {
    std::lock_guard<std::mutex> lock(orderedReadMtx);
    if (!pendingOrderedReads.empty()) {
        next = std::move(pendingOrderedReads.front());
        pendingOrderedReads.pop();
        return true;
    }
    orderedReadInFlight = false;
    return false;
}

Client::~Client() {
    g_clientDestructions.fetch_add(1, std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lock(g_clientLifeLogMtx);
        std::cout << "[ClientLife] sessionId=" << sessionID
                  << " ~Client totals: asyncClose=" << g_asyncCloseCompletions.load()
                  << " dtor=" << g_clientDestructions.load() << std::endl;
    }
}

std::uint64_t Client::debugAsyncCloseCompletions() noexcept {
    return g_asyncCloseCompletions.load(std::memory_order_relaxed);
}

std::uint64_t Client::debugClientDestructions() noexcept {
    return g_clientDestructions.load(std::memory_order_relaxed);
}
