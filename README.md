<p align="center">
  <img src="reactChatRoom/public/favicon.png" alt="ChatMe logo" width="96" />
</p>

# ChatMe

Messaging platform for **1:1 messaging between peers**.

**Live demo:** [https://connectroom.duckdns.org](https://connectroom.duckdns.org)

## Screenshots

| Login | Chat |
|-------|------|
| _Add login screenshot_ | _Add chat screenshot_ |

_Add a conversation screenshot after you capture it (login + chat list + active chat)._

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

## Architecture (overview)

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

**Project structure**

- `chat_room_server/` — C++ WebSocket server  
- `reactChatRoom/` — React + Capacitor client  
- `media_storage/` — media HTTP service  

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
2. Apply the PostgreSQL schema (migration tooling / committed schema dump planned — not in this repo yet).  
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


## License

Personal portfolio project. Ask before reusing commercially.
