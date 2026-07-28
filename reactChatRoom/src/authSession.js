/**
 * Login: rememberMe → localStorage; otherwise sessionStorage (per tab). Logout clears both.
 * Each tab records scope in sessionStorage (`tab` | `local`) so JWT reads do not cross wires
 * when another tab sets remember me in localStorage.
 */

const USER_KEY = 'chatSessionUser'
/** Per-tab: which storage this tab's WS traffic should use (`tab` = sessionStorage, `local` = remember me). */
const TAB_AUTH_SCOPE_KEY = 'chatAuthScope'

/**
 * @param {boolean} rememberMe
 * @returns {Storage}
 */
export function getAuthStore(rememberMe) {
  return rememberMe ? localStorage : sessionStorage
}

function clearAuthFromStorage(store) {
  store.removeItem('jwt')
  store.removeItem('sessionId')
  store.removeItem(USER_KEY)
}

function hasValidAuthPair(store) {
  return Boolean(store.getItem('jwt') && store.getItem(USER_KEY))
}

/**
 * Which storage this tab should use for JWT + restore.
 * @returns {Storage | null}
 */
function resolveAuthStoreForTab() {
  const scope = sessionStorage.getItem(TAB_AUTH_SCOPE_KEY)

  if (scope === 'tab') {
    return hasValidAuthPair(sessionStorage) ? sessionStorage : null
  }
  if (scope === 'local') {
    return hasValidAuthPair(localStorage) ? localStorage : null
  }

  // New tab / before first scoped login: ephemeral session wins, then remembered local.
  if (hasValidAuthPair(sessionStorage)) return sessionStorage
  if (hasValidAuthPair(localStorage)) return localStorage
  return null
}

/**
 * @param {Storage} store
 * @returns {{ userName: string, fullName: string, userId: number | null, token: string } | null}
 */
function readRestoredSessionFrom(store) {
  const jwt = store.getItem('jwt')
  const raw = store.getItem(USER_KEY)

  if (!jwt || !raw) {
    if (raw) store.removeItem(USER_KEY)
    return null
  }

  try {
    const u = JSON.parse(raw)
    if (u == null || typeof u !== 'object') return null
    const userId = u.userId != null ? Number(u.userId) : null
    const userName = u.userName != null ? String(u.userName) : ''
    const fullName = u.fullName != null ? String(u.fullName) : ''
    if (!userName && userId == null) return null
    return {
      userName,
      fullName,
      userId,
      token: jwt,
    }
  } catch {
    clearAuthFromStorage(store)
    return null
  }
}

/**
 * @param {{ token: string | null, sessionId: string | number, profile: { userName: string, fullName: string, userId: number | null }, rememberMe: boolean }} opts
 */
export function writeAuthAfterLogin({ token, sessionId, profile, rememberMe }) {
  if (rememberMe) {
    clearAuthFromStorage(sessionStorage)
    sessionStorage.setItem(TAB_AUTH_SCOPE_KEY, 'local')
    clearAuthFromStorage(localStorage)
    const target = localStorage
    if (token) target.setItem('jwt', token)
    target.setItem('sessionId', String(sessionId))
    target.setItem(USER_KEY, JSON.stringify({
      userName: profile.userName,
      fullName: profile.fullName,
      userId: profile.userId,
    }))
    return
  }

  clearAuthFromStorage(localStorage)
  sessionStorage.setItem(TAB_AUTH_SCOPE_KEY, 'tab')
  clearAuthFromStorage(sessionStorage)
  const target = sessionStorage
  if (token) target.setItem('jwt', token)
  target.setItem('sessionId', String(sessionId))
  target.setItem(USER_KEY, JSON.stringify({
    userName: profile.userName,
    fullName: profile.fullName,
    userId: profile.userId,
  }))
}

/** JWT for this tab only (respects tab auth scope). */
export function getStoredJwt() {
  const store = resolveAuthStoreForTab()
  return store?.getItem('jwt') ?? null
}

/** Restore on load for this tab only. */
export function readRestoredSession() {
  const store = resolveAuthStoreForTab()
  return store ? readRestoredSessionFrom(store) : null
}

export function clearSessionAuth() {
  clearAuthFromStorage(localStorage)
  clearAuthFromStorage(sessionStorage)
  sessionStorage.removeItem(TAB_AUTH_SCOPE_KEY)
}

/** After SESSION_INIT + re-bind, keep stored session id aligned with the live socket (active store only). */
export function patchStoredSessionId(sessionId) {
  const store = resolveAuthStoreForTab()
  if (!store) return
  store.setItem('sessionId', String(sessionId))
}
