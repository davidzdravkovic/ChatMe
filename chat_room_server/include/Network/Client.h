#ifndef CLIENT_H
#define CLIENT_H
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include "../Deserialization/DTO/RequestStruct.h"
#include <atomic>
#include <chrono>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <cstdint>



enum class ClientState {
    Active,
    Closing,
    Closed
};

using WsStream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;

class Client {
    int sessionID;
    //destructor = ws.close
    std::shared_ptr<WsStream> ws;
    std::atomic<ClientState> state{ClientState::Active};
    std::queue<std::vector<uint8_t>> messagesToSend;
    std::mutex sendQueueMutex;
    std::atomic<bool> isWriting{false};
    bool logOutPending{false};
    std::atomic<bool> authRejectClose{false};
    std::queue<RequestStruct> pendingOrderedReads;
    std::mutex orderedReadMtx;
    bool orderedReadInFlight = false;

public:
    explicit Client(std::shared_ptr<WsStream> wsStream, int iD);
    ~Client();

    int getId() { return sessionID; }
    WsStream& getWs() { return *ws; }
    std::atomic<ClientState>& getState() { return state; }
    /// @param self same as `this` — call as `c->beginClose(c, ...)` so the `shared_ptr` outlives async_close.
    void beginClose(std::shared_ptr<Client> self, std::function<void(int)> onClosed);
    void enqueueMessage(const std::vector<uint8_t> message);
    void finishInFlight();
    bool hasMessagesToSend();
    std::vector<uint8_t> dequeueMessage();
    bool notInFlight();
    bool getLogOutPending() const { return logOutPending;}
    void setLogOutPending(bool pending) { logOutPending = pending;}
    bool getAuthRejectClose() const { return authRejectClose.load(std::memory_order_acquire); }
    void setAuthRejectClose(bool pending) { authRejectClose.store(pending, std::memory_order_release); }
    bool blocksIngress() const {
        return logOutPending || authRejectClose.load(std::memory_order_acquire);
    }

    bool tryAcquireOrderedRead(const RequestStruct& req);
    bool releaseOrderedRead(RequestStruct& next);

    /// `async_close` completion count (per process, for tests / logs).
    static std::uint64_t debugAsyncCloseCompletions() noexcept;
    /// `~Client` call count (per process).
    static std::uint64_t debugClientDestructions() noexcept;
};

#endif
