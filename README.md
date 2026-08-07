<p align="center">
  <img src="reactChatRoom/public/favicon.png" alt="ChatMe logo" width="120" />
</p>

<h1 align="center">ChatMe</h1>

<p align="center">
  Messaging platform for <strong>1-to-1 private chat</strong>
</p>

<p align="center">
  <a href="https://connectroom.duckdns.org"><strong>Live app → connectroom.duckdns.org</strong></a>
</p>

## Screenshot

<p align="center">
  <img
    src="readme-screenshots/Chat_screenshot.png"
    alt="ChatMe chat interface"
    width="900"
  />
</p>

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

#### API server (`chat_room_server`)

- C++17 toolchain
- CMake 3.20+
- Dependencies:
  - Boost 1.89.0
  - PostgreSQL 17.5
  - libpqxx 7.10.1
  - OpenSSL 3.6.2
  - libsodium 1.0.20
  - jwt-cpp 0.7.2 (Windows) / 0.7.1 (Linux/BSD)
  - nlohmann/json 3.11.3

#### Database

- PostgreSQL 17.5

#### Client

- React + Node.js 18+

#### Media server

- C++17 toolchain
- CMake 3.20+
- OpenSSL 3.0.17
- cpp-httplib (vendored locally in the project tree)

> **Note:** The versions listed above are the versions used and tested with this project. They are recommended for compatibility but are not strict version requirements.

### Database

1. Create a database/user.  
2. Apply the PostgreSQL schema (migration tooling / committed schema dump planned — not in this repo yet).  
3. Point server config / env at that database (`chat_room_server/Config/`, env example in `chat_room_server/deploy/chat-server.env.example`).

## Build

> `Note`: Before configuring and building, make sure all required dependencies are installed and discoverable by CMake.

### API server

```bash 
# During CMake configuration: # - Linux/BSD: jwt-cpp is fetched automatically when JWT support is enabled. # - All platforms: nlohmann/json is downloaded into third_party/.
cd chat_room_server
cmake -S . -B build
cmake --build build
cd build
.\chat_server
```


### Media server

```bash 
cd media_storage
cmake -S . -B build
cmake --build build
cd build
.\media_server
```

### Client

```bash
cd reactChatRoom
npm install
npm run dev
```


## License

Personal portfolio project. Ask before reusing commercially.
