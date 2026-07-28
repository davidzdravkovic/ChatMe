# ConnectRoom (ChatRoom)

Messaging platform for **1:1 messaging between peers**. 

**Live demo:** [https://connectroom.duckdns.org](https://connectroom.duckdns.org)

> Add screenshots or a short GIF here after you capture them (login + open chat + send message).

---

## Why this project exists

The main purpose of this project was to build a **real product for real users**.

The second goal was to develop a deeper understanding of server-side networking by building a **custom C++ WebSocket backend** (Boost.Asio / Boost.Beast) without relying on a high-level framework. That let me own the network layer: connections, message flow, and how a server handles concurrent real-time traffic.
 
 ---

## Features

**Accounts**
- User registration and login

**Messaging**
- 1-to-1 private chats
- Real-time text messaging
- Reply to a message
- Message pagination and in-chat search

**Media**
- Image messages
- Profile pictures

**Live signals**
- Typing indicators
- Read receipts
- Emoji reactions
- Online and last-active presence

**Discovery**
- Recent chats
- User search

---

## Tech stack

| Layer | Technologies |
|--------|----------------|
| Frontend | React 18 · Vite · Capacitor 8 · JavaScript |
| Backend | C++17 · Boost.Asio/Beast (WebSockets) · cpp-httplib (media) |
| Data | PostgreSQL · libpqxx |
| Auth | JWT · Argon2id (libsodium) |
| Deploy | Linux VPS · nginx · systemd |

---

## Architecture (one look)

```text
Browser / Capacitor
        │
        ▼
   nginx (HTTPS)
     ├── static React build
     ├── /ws     → C++ chat server  (127.0.0.1:12346)
     └── /media/ → media service    (:8081)
                      │
                 PostgreSQL
```

Deeper map: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) · Deploy overview: [`docs/DEPLOYMENT_OVERVIEW.md`](docs/DEPLOYMENT_OVERVIEW.md)

**Repos layout**

- `chat_room_server/` — C++ WebSocket server  
- `reactChatRoom/` — React + Capacitor client  
- `media_storage/` — media HTTP service  
- `docs/` — architecture, deploy notes, portfolio notes  

---

## Protocol (high level)

Client and server talk **JSON over one WebSocket**. Typical flow:

1. Connect → server sends `SESSION_INIT` with `sessionId`
2. `LOGIN_REQUEST` / `CREATE_REQUEST` (no token) → JWT on success  
3. Later messages include session id + JWT  
4. Live traffic examples: `MESSAGE_REQUEST`, `FETCH_MESSAGES_REQUEST`, `TYPING_REQUEST`, `SEEN_REQUEST`, `REACTION_REQUEST`, media upload/finalize requests  

Shared enums: `chat_room_server/common/RequestType.h`, `ResponseType.h`

---

## Quick start (local development)

Exact dependency versions and OS paths vary. Use this as the map; adjust for your machine.

### Prerequisites

- PostgreSQL  
- C++17 toolchain + CMake (Boost, libpqxx, OpenSSL; JWT/libsodium as configured)  
- Node.js 18+ for the client  
- Media service buildable from `media_storage/`  

### Database

1. Create a database/user.  
2. Apply schema from `chat_room_server/prod-schema.sql` or `local-schema.sql`.  
3. Point server config / env at that database (`chat_room_server/Config/`, env example in `chat_room_server/deploy/chat-server.env.example`).

### Server

```bash
cd chat_room_server
# Configure & build with CMake (see CMakePresets.json / deploy notes)
# Prefer a JWT-enabled build for anything beyond local experiments:
#   cmake .. -DCHAT_ENABLE_JWT=ON ...
# Set JWT_SECRET in the environment (see deploy/chat-server.env.example)
./chat_server   # or your build output name
```

Default listen target in production is behind nginx → `127.0.0.1:12346`.

### Media service

Build and run `media_storage` on the port nginx expects (production uses `:8081`).

### Client

```bash
cd reactChatRoom
npm install
# Dev: set VITE_WS_URL / VITE_MEDIA_BASE for local (e.g. ws://localhost:...)
npm run dev
```

Production build:

```bash
npm run build
# Serve dist/ via nginx; see reactChatRoom/deploy/
```

Android:

```bash
npm run cap:sync
npm run cap:open
```

### Tests (client)

```bash
cd reactChatRoom
npm run test:run
```

Server automated tests are a planned improvement (see below).

---

## Production deploy (summary)

- **nginx** terminates TLS, serves the SPA, proxies `/ws` and `/media/`  
- **systemd** runs the chat server (`chat_room_server/deploy/chat-server.service`)  
- Secrets via EnvironmentFile (`JWT_SECRET`, DB credentials)  
- Details: [`docs/DEPLOYMENT_OVERVIEW.md`](docs/DEPLOYMENT_OVERVIEW.md), `reactChatRoom/deploy/HTTPS-SETUP.txt`

---

## What I’d improve next

Honest gaps (engineering maturity, not excuses):

1. Broader **automated tests** on the C++ server + CI  
2. Harden **JWT defaults** (always-on verification in prod builds; honor configured TTL)  
3. Remove leftover **debug logging** on hot paths  
4. **Schema/migration** discipline so dumps always match the live DB  
5. Optional: **TypeScript** on the client for common junior web job filters  

---

## Interview / discussion hooks

Worth being ready to explain:

- WebSocket session lifecycle and reconnect re-bind  
- Why identity is bound from JWT (`bindTrustedUser`) not client-supplied sender ids  
- Ordered-message handling under concurrent reads  
- Conversation “epoch” / stale-fetch guards on the client  
- Multi-phase image upload (WS + HTTP media)

---

## License

Personal portfolio project. Ask before reusing commercially.
