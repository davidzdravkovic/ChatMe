import { devError, devLog, devWarn } from '../utils/logger'
import { getStoredJwt } from '../authSession'

/** Dev: direct to C++. Behind nginx: e.g. VITE_WS_URL=ws://localhost/ws */
const WS_URL = import.meta.env.VITE_WS_URL || 'ws://localhost:12346/';
let connectsAttempts = 0
let connectsFailed = 0
let ws = null;
let isReady = false;
let sessionId = null;
let hasReceivedSessionId = false;

const messageListeners = new Set();
const connectionCallback = new Set();
const disconnectionListeners = new Set();

function resetConnectionState() {
  isReady = false
  sessionId = null
  hasReceivedSessionId = false
}

function bindSocketHandlers(socket) {
  //Handlers are scheduled for socket at the moment of creation, they close the module WS variable which points always to the latest socket.
  //So the guard ws !== socketRef checks the frozen socket reference `socketRef` against the latest socket `ws` to avoid handling events from old sockets after refresh/logout. 
  const socketRef = socket

  socket.onopen = () => {
    if (ws !== socketRef) return
    isReady = true
    devLog('WebSocket connected')
  }

  socket.onmessage = (event) => {
    if (ws !== socketRef) return
    if (!hasReceivedSessionId) {
      if (typeof event.data !== 'string') {
        devWarn('Expected text SESSION_INIT frame, got:', event.data)
        return
      }
      let msg
      try {
        msg = JSON.parse(event.data)
      } catch {
        return
      }
      if (msg.type === 'SESSION_INIT' && msg.sessionId != null) {
        sessionId = Number(msg.sessionId)
        hasReceivedSessionId = true
        devLog('Session ID received:', sessionId)
        connectionCallback.forEach((cb) => cb())
      }
      return
    }

    handleGeneralMessage(event)
  }

  socket.onclose = () => {
    if (ws !== socketRef) return
    resetConnectionState()
    ws = null
    // This callback is creating new connection, cause on close can happen in situation when:
    // 1. User is logging out
    // 2. Refreshing page
    //So every onclose needs  => connect() for to keep the invariant always connected to the server in every app state  
    disconnectionListeners.forEach((cb) => cb())
    devLog('WebSocket closed')
  }

  socket.onerror = (err) => {
    if (ws !== socketRef) return
    isReady = false
    devError('WebSocket error', err)
  }
}

/**
 * Opens or reuses the WebSocket. Call after logout/server close so login/signup can get SESSION_INIT again.
 * Old sockets in CLOSING/CLOSED do not block opening a new one; stale onclose cannot clear a newer socket.
 */
export function connect() {
  //Even if ws is not null it could be in process of closing and onClose is not called yet.
console.log(`Connecting WebSocket... ${++connectsAttempts}`)
  if (ws) {
    const state = ws.readyState
    if (state === WebSocket.OPEN || state === WebSocket.CONNECTING) {
      ++connectsFailed
       devLog(`Already exists the number of attempts to connect is ${connectsAttempts} and the number of failed attempts is ${connectsFailed}`)
      return ws
    }
  }
  //If there is old socket is not going to clear the state because of the guard on stale sockets, so for that reason the clearing of the state is managed before the creation
  resetConnectionState()
  const socket = new WebSocket(WS_URL)
  ws = socket
  bindSocketHandlers(socket)
  return ws
}

/** Gracefully closes the current socket (if any). */
export function disconnect(code = 1000, reason = 'logout') {
  //Nothing to close
  if (!ws) return
  const state = ws.readyState
  //Disconnect called on socket in process of closing
  if (state === WebSocket.CLOSING || state === WebSocket.CLOSED) return
  try {
    //Actuall close which is going to finish with onclose handler which is async and can be called after another connect() so for that reason there is a guard for stale calles.
    ws.close(code, reason)
  } catch (err) {
    devWarn('WebSocket close failed:', err)
  }
}

// ---- Handle general messages ----
function handleGeneralMessage(event) {
  messageListeners.forEach((cb) => cb(event));
}

// ---- Send JSON over WebSocket (optional JWT; sessionId stays in the DTO) ----
export function sendMessage(data, { attachToken = true } = {}) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    const token = getStoredJwt()

    let payload;

    if (typeof data === 'object') {
      payload = attachToken && token ? { ...data, token } : data;
    } else {
      try {
        const parsed = JSON.parse(data);
        payload = attachToken && token ? { ...parsed, token } : parsed;
      } catch {
        payload = data;
      }
    }

    ws.send(
      typeof payload === 'string' ? payload : JSON.stringify(payload)
    );
  } else {
    devWarn('WebSocket not open, cannot send message');
  }
}

// ---- Accessors ----
export function getIsReady() {
  return isReady;
}

export function getSessionId() {
  return sessionId;
}


// ---- Subscribe to messages ----
export function subscribeMessages(callback) {
  messageListeners.add(callback);
  return () => messageListeners.delete(callback);
}

export function subscribeConnection(callback) {
  //Subcriber puts the callback in the set and after the session ID is received and the callback is called.
  connectionCallback.add(callback)
  //All callbacks are already fired so in case the subscriber is late, subscriber's callback is called immediately.
  if (hasReceivedSessionId) {
    callback()
  }

  return () => connectionCallback.delete(callback)
}

/** Fires when the socket closes or is replaced; use to clear UI and call connect() again. */
export function subscribeDisconnection(callback) {
  disconnectionListeners.add(callback)
  return () => disconnectionListeners.delete(callback)
}
