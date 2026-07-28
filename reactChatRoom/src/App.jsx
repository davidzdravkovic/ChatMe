import { useState, useEffect, useRef, useCallback } from 'react'
import { flushSync } from 'react-dom'
import { connect, disconnect, sendMessage, getSessionId, subscribeConnection, subscribeDisconnection, subscribeMessages } from './network/wsConnection'
import { createLogoutRequest, createAuthRequest } from './Dto/dto'
import { readRestoredSession, clearSessionAuth, getStoredJwt, patchStoredSessionId } from './authSession'
import ChatConnectSkeleton from './components/ChatConnectSkeleton/ChatConnectSkeleton'
import LoginPage from './pages/LoginPage'
import SignUpPage from './pages/SignUpPage'
import ChatPage from './pages/ChatPage'

const AUTH_ERROR_CODES = new Set(['UNAUTHORIZED', 'AUTH_FAILED', 'SESSION_MISMATCH'])

function errorResponseCode(msg) {
  if (!msg || msg.request !== 'ERROR_RESPONSE') return null
  const data = msg.data
  if (!data) return null
  if (typeof data.code === 'string') return data.code
  if (Array.isArray(data) && typeof data[0]?.code === 'string') return data[0].code
  return null
}

function App() {
  // Restore via authSession: each tab is bound to tab-scoped or remember-me storage (chatAuthScope).
  // currentUser is also set directly after login/signup without going through restore.
  const [currentUser, setCurrentUser] = useState(() => readRestoredSession())
  const [page, setPage] = useState(currentUser ? 'chat' : 'login')
  const [wsSessionReady, setWsSessionReady] = useState(false)
  const connectRef = useRef(0)
  /** True after disconnect/restore: wait for AUTH_RESPONSE before chat. False after fresh login/signup. */
  const pendingRebindRef = useRef(Boolean(readRestoredSession()))
  /** Skip one auto-reconnect after server auth failure (ERROR_RESPONSE / failed rebind). */
  const skipReconnectRef = useRef(false)

  const handleAuthSessionInvalid = useCallback(() => {
    skipReconnectRef.current = true
    pendingRebindRef.current = false
    clearSessionAuth()
    setCurrentUser(null)
    setPage('login')
    setWsSessionReady(false)
  }, [])

  useEffect(() => {
    const unsub = subscribeMessages((event) => {
      if (typeof event.data !== 'string') return
      let msg
      try {
        msg = JSON.parse(event.data)
      } catch {
        return
      }
      const code = errorResponseCode(msg)
      if (code && AUTH_ERROR_CODES.has(code)) {
        handleAuthSessionInvalid()
      }
    })
    return unsub
  }, [handleAuthSessionInvalid])

  useEffect(() => {
    connect()
    connectRef.current += 1
    console.log(`App mounted, connecting ws (count: ${connectRef.current})`)
  
  }, [])


  // Chat: SESSION_INIT → optional re-bind (AUTH) → then mount ChatPage.
  useEffect(() => {
    if (page !== 'chat' || !currentUser) {
      setWsSessionReady(false)
      return
    }

    let cancelled = false
    let unsubAuthMessages = () => {}

    const finishReady = () => {
      if (!cancelled) setWsSessionReady(true)
    }

    const failRebind = () => {
      unsubAuthMessages()
      unsubAuthMessages = () => {}
      handleAuthSessionInvalid()
    }

    const startRebind = () => {
      setWsSessionReady(false)
      unsubAuthMessages()

      unsubAuthMessages = subscribeMessages((event) => {
        let msg
        try {
          msg = typeof event.data === 'string' ? JSON.parse(event.data) : event.data
        } catch {
          return
        }
        if (!msg || msg.response !== 'AUTH_RESPONSE') return

        unsubAuthMessages()
        unsubAuthMessages = () => {}

        if (msg.status === 'SUCCESS') {
          const sid = getSessionId()
          if (sid != null) patchStoredSessionId(sid)
          const row = msg.data?.[0]
          flushSync(() => {
            setCurrentUser((prev) =>
              prev
                ? {
                    ...prev,
                    ...(row?.profileUrl ? { profileUrl: row.profileUrl } : {}),
                  }
                : prev,
            )
          })
          pendingRebindRef.current = false
          finishReady()
        } else {
          failRebind()
        }
      })

      sendMessage(JSON.stringify(createAuthRequest()))
    }

    const onSessionInit = () => {
      if (pendingRebindRef.current) {
        startRebind()
      } else {
        finishReady()
      }
    }

    const unsubConn = subscribeConnection(onSessionInit)

    const unsubDisc = subscribeDisconnection(() => {
      if (skipReconnectRef.current) {
        skipReconnectRef.current = false
        return
      }
      pendingRebindRef.current = true
      setWsSessionReady(false)
      unsubAuthMessages()
      unsubAuthMessages = () => {}
      connect()
    })

    return () => {
      cancelled = true
      unsubConn()
      unsubDisc()
      unsubAuthMessages()
    }
  }, [page, currentUser?.userId, handleAuthSessionInvalid])

  //Sanity check if somehow the page is `chat` without a user, redirect to login page.
  useEffect(() => {
    if (page === 'chat' && !currentUser) {
      setPage('login')
    }
  }, [page, currentUser])



//Render components based on the page state.
  if (page === 'login') {
    return (
      <LoginPage
        onNavigateToSignup={() => setPage('signup')}
        onLogin={(userData) => {
          pendingRebindRef.current = false
          setCurrentUser(userData)
          setPage('chat')
        }}
      />
    )
  }

  if (page === 'signup') {
    return (
      <SignUpPage
        onNavigateToLogin={() => setPage('login')}
          onLogin={(userData) => {
          pendingRebindRef.current = false
          setCurrentUser(userData)
          setPage('chat')
        }}
      />
    )
  }

  if (page === 'chat' && currentUser && !wsSessionReady) {
    return (
      <div className="app-chat-wrapper">
        <ChatConnectSkeleton label={pendingRebindRef.current ? 'Reconnecting' : 'Connecting'} />
      </div>
    )
  }

  return (
    <div className="app-chat-wrapper">
      <ChatPage
        currentUser={currentUser}
        onProfileUrl={(profileUrl) => {
          if (!profileUrl) return
          setCurrentUser((prev) => (prev ? { ...prev, profileUrl } : prev))
        }}
        onLogout={() => {
          const token = getStoredJwt()
          const sid = getSessionId()
          if (token && sid != null) {
            sendMessage(JSON.stringify(createLogoutRequest()))
          }
          //Disconnect makes a reconnect as a side effect
          disconnect()
          clearSessionAuth()
          setCurrentUser(null)
          setPage('login')
        }}
      />
    </div>
  )
}

export default App
