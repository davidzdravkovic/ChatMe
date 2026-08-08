# Architecture — chat server

How a WebSocket frame becomes a database write and a response. Product overview and setup: [README.md](README.md).

The chat server keeps **I/O light on Asio network threads** and runs **business logic + DB on worker threads**.

## Request path

```mermaid
%%{init: {
  "theme": "base",
  "themeVariables": {
    "fontSize": "15px",
    "lineColor": "#94a3b8",
    "clusterBkg": "#f8fafc",
    "clusterBorder": "#cbd5e1",
    "primaryBorderColor": "#cbd5e1"
  },
  "flowchart": {
    "curve": "linear",
    "nodeSpacing": 48,
    "rankSpacing": 70,
    "padding": 32
  }
}}%%
flowchart TD
    IN(["Client · inbound JSON"])

    subgraph ING["Ingress"]
        READ["async_read"]
        PARSE["RequestParser"]
        AUTH["WsAuthMiddleware"]
        GATE["TrafficReadPolicy"]
        READ --> PARSE --> AUTH --> GATE
    end

    ROUTE{{"TrafficController"}}

    subgraph FASTLANE["Fast — LOGIN"]
        direction LR
        QF["queue"] --> WF["2 workers"]
    end

    subgraph SLOWLANE["Slow — other"]
        direction LR
        QS["queue"] --> WS["7 workers"]
    end

    HANDLER["Handler · routeToManager"]
    DB[("PostgreSQL")]
    OUT["async_write"]
    DONE(["Client · response"])

    IN --> READ
    GATE --> ROUTE
    ROUTE --> QF
    ROUTE --> QS
    WF --> HANDLER
    WS --> HANDLER
    HANDLER <--> DB
    HANDLER --> OUT --> DONE

    classDef endpoint fill:#0f172a,stroke:#0f172a,color:#f8fafc
    classDef io fill:#f0f9ff,stroke:#38bdf8,stroke-width:1.5px,color:#0c4a6e
    classDef auth fill:#f5f3ff,stroke:#a78bfa,stroke-width:1.5px,color:#4c1d95
    classDef route fill:#faf5ff,stroke:#8b5cf6,stroke-width:2px,color:#4c1d95
    classDef fast fill:#ecfdf5,stroke:#34d399,stroke-width:1.5px,color:#065f46
    classDef slow fill:#fff1f2,stroke:#fb7185,stroke-width:1.5px,color:#9f1239
    classDef app fill:#f8fafc,stroke:#94a3b8,stroke-width:1.5px,color:#0f172a
    classDef db fill:#fffbeb,stroke:#fbbf24,stroke-width:1.5px,color:#78350f

    class IN,DONE endpoint
    class READ,PARSE,OUT io
    class AUTH,GATE auth
    class ROUTE route
    class QF,WF fast
    class QS,WS slow
    class HANDLER app
    class DB db

    linkStyle default stroke:#94a3b8,stroke-width:1.6px
```

_Ingress runs on Boost.Asio `io_context` network threads. Lane titles encode LOGIN vs other traffic — edge labels are omitted to keep the layout clean._

**Stages**

1. **Accept / session** — `Network` accepts the WebSocket, assigns a `sessionId`, sends `SESSION_INIT`, starts `readWs` (`Network.cpp`).
2. **Ingress pipeline** — each frame runs `RequestPipeline::run`: parse JSON → `WsAuthMiddleware` → ordered-read gate → then `TrafficController::route` (`RequestPipeline.cpp`).
3. **Lanes** — `TrafficPolicy` chooses a queue (`TrafficPolicy.h`):
   - **Fast:** `LOGIN_REQUEST` (2 worker threads)
   - **Slow:** everything else (7 worker threads), including `MESSAGE_REQUEST` / `FIRST_MESSAGE_REQUEST`
4. **Dispatch** — workers `pop` FIFO from `SharedContext`, call `Handler::routeToManager`, then schedule `NetworkAction`s back onto the `io_context` for `async_write` (`Dispatcher.cpp`).
5. **Writes** — one in-flight write per client; payloads queue until the previous `async_write` finishes (`writeWs`).

## JWT on the path

| Step | Where | What |
|------|--------|------|
| Anonymous | `LOGIN_REQUEST` / `CREATE_REQUEST` | No token; `WsAuthMiddleware` lets them through |
| Issue | `Handler` after successful login/create/auth | `Auth::issueJWT` → `JWT_SECRET` (`getenv`) |
| Verify | Every other request on ingress | `WsAuthMiddleware` → `Auth::verify` before the request is queued |
| Media | Signed URL tokens | Same `JWT_SECRET` on `chat_server` and `media_server` |

`sessionId` must match the connection; mismatch closes the session with `SESSION_MISMATCH`.

## Ordering (send path)

TCP and a **single outstanding `async_read` per client** preserve request order into the shared queues. Worker **completion** order is not guaranteed across threads.

For types marked `requires_ordering` (`MESSAGE_REQUEST`, `FIRST_MESSAGE_REQUEST`), `TrafficReadPolicy` + a per-client ordered gate ensure the next ordered request for that client is not dispatched until the previous one’s response path releases the gate (`RELEASE_ORDERED_GATE`). Unordered types (e.g. typing) can still progress without waiting on a slow insert.

Deep dive: [Ordering Messages in a Concurrent C++ WebSocket Server](https://davidzdravkovic.github.io/posts/message-ordering.html).

## Protocol (high level)

Client and server talk **JSON over one WebSocket**. Typical flow:

1. Connect → server sends `SESSION_INIT` with `sessionId`
2. `LOGIN_REQUEST` / `CREATE_REQUEST` (no token) → JWT on success
3. Later messages include session id + JWT
4. Live traffic examples: `MESSAGE_REQUEST`, `FETCH_MESSAGES_REQUEST`, `TYPING_REQUEST`, `SEEN_REQUEST`, `REACTION_REQUEST`, media upload/finalize requests

Shared enums: `chat_room_server/common/RequestType.h`, `ResponseType.h`
