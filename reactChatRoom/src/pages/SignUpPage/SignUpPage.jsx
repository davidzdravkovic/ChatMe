import './SignUpPage.css'
import { useState, useEffect } from 'react'
import { createCreateStruct } from '../../Dto/dto'
import {
  connect,
  getSessionId,
  sendMessage,
  subscribeMessages,
  subscribeConnection,
  subscribeDisconnection,
} from '../../network/wsConnection'
import { writeAuthAfterLogin } from '../../authSession'

function SignUpPage({ onNavigateToLogin, onLogin }) {
  const [connection, setConnection] = useState(false)
  const [connectionError, setConnectionError] = useState(false)

  useEffect(() => {
    connect()
    const unsubConn = subscribeConnection(() => {
      setConnection(true)
      setConnectionError(false)
    })
    const unsubDisc = subscribeDisconnection(() => {
      setConnection(false)
      connect()
    })
    return () => {
      unsubConn()
      unsubDisc()
    }
  }, [])

  const handleSubmit = (e) => {
    e.preventDefault()

    if (!connection) {
      setConnectionError(true)
      return
    }

    const fullName = e.target.fullName.value.trim()
    const username = e.target.username.value.trim()
    const email = e.target.email.value.trim()
    const password = e.target.password.value
    const confirmPassword = e.target.confirmPassword.value

    if (!fullName) {
      alert('Full name is required')
      return
    }
    if (!username) {
      alert('Username is required')
      return
    }
    if (!email) {
      alert('Email is required')
      return
    }
    if (!password) {
      alert('Password is required')
      return
    }
    if (password !== confirmPassword) {
      alert('Passwords do not match')
      return
    }

    const rememberMe = e.target.rememberMe?.checked === true
    const createDTO = createCreateStruct(username, password, fullName, email)

    const unsubscribe = subscribeMessages((payload) => {
      let msg
      try {
        msg = typeof payload.data === 'string' ? JSON.parse(payload.data) : payload.data
      } catch {
        return
      }
      if (!msg) return

      const isSuccess = msg.response === 'CREATE_RESPONSE' && msg.status === 'SUCCESS'
      const isFailure = msg.response === 'CREATE_RESPONSE' && msg.status !== 'SUCCESS'

      if (isSuccess) {
        const userData = Array.isArray(msg.data) && msg.data[0] ? msg.data[0] : {}
        const token = userData.token
        const nextUser = {
          userName: userData.userName ?? username,
          fullName: userData.name ?? fullName,
          userId: userData.userId != null ? Number(userData.userId) : null,
          token: token ?? null,
        }
        writeAuthAfterLogin({
          token: token ?? null,
          sessionId: getSessionId(),
          rememberMe,
          profile: {
            userName: nextUser.userName,
            fullName: nextUser.fullName,
            userId: nextUser.userId,
          },
        })
        onLogin?.(nextUser)
        unsubscribe()
      } else if (isFailure) {
        const err = msg.data?.[0]?.error ?? msg.error ?? 'Could not create account'
        alert('Sign up failed: ' + err)
        unsubscribe()
      }
    })

    sendMessage(JSON.stringify(createDTO), { attachToken: false })
  }

  return (
    <div className="signup-page">
      <div className="signup-left">
        <div className="auth-hero">
          <div className="auth-logo-wrap">
            <img src="/favicon.png" alt="" className="auth-logo-icon" aria-hidden="true" />
          </div>
          <h1 className="auth-brand">ChatMe</h1>
          <p className="auth-tagline">Create your account</p>
        </div>
      </div>
      <div className="signup-right">
        <div className="signup-card">
          <h2 className="signup-title">Create account</h2>
          <p className="signup-subtitle">Welcome onboard</p>

          <form className="signup-form" onSubmit={handleSubmit}>
            <div className="signup-field">
              <label className="signup-label" htmlFor="fullName">
                Full name
              </label>
              <input
                className="signup-input"
                id="fullName"
                name="fullName"
                placeholder="Enter your full name"
                autoComplete="name"
              />
            </div>
            <div className="signup-field">
              <label className="signup-label" htmlFor="username">
                Username
              </label>
              <input
                className="signup-input"
                id="username"
                name="username"
                placeholder="Choose a username"
                autoComplete="username"
              />
            </div>
            <div className="signup-field">
              <label className="signup-label" htmlFor="email">
                Email
              </label>
              <input
                className="signup-input"
                id="email"
                name="email"
                type="email"
                placeholder="you@example.com"
                autoComplete="email"
              />
            </div>
            <div className="signup-field">
              <label className="signup-label" htmlFor="password">
                Password
              </label>
              <input
                className="signup-input"
                id="password"
                name="password"
                type="password"
                placeholder="••••••••"
                autoComplete="new-password"
              />
            </div>
            <div className="signup-field">
              <label className="signup-label" htmlFor="confirmPassword">
                Confirm password
              </label>
              <input
                className="signup-input"
                id="confirmPassword"
                name="confirmPassword"
                type="password"
                placeholder="••••••••"
                autoComplete="new-password"
              />
            </div>
            <div className="signup-row signup-remember">
              <label className="signup-check">
                <input type="checkbox" id="rememberMe" name="rememberMe" />
                <span>Remember me on this device</span>
              </label>
            </div>
            <button type="submit" className="signup-btn">
              Create account
            </button>
            {connectionError && (
              <p className="signup-error" role="alert">
                Connection problem. Check your network and try again.
              </p>
            )}
          </form>

          <p className="signup-switch">
            Already have an account?{' '}
            <a
              href="#"
              className="signup-link"
              onClick={(e) => {
                e.preventDefault()
                onNavigateToLogin?.()
              }}
            >
              Sign in
            </a>
          </p>
        </div>
      </div>
    </div>
  )
}

export default SignUpPage
