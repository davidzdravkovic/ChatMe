import { useEffect, useRef, useState } from 'react'
import './ChatMessageSearch.css'
import { formatMessageSearchDate } from '../../utils/formatTime'
import { getAvatarColor } from '../../utils/avatarColor'

const SEARCH_DEBOUNCE_MS = 300

function highlightSnippet(content, query) {
  const text = String(content ?? '')
  const q = String(query ?? '').trim()
  if (!q) return text
  const idx = text.toLowerCase().indexOf(q.toLowerCase())
  if (idx === -1) return text
  // Keep a little context around the first match.
  const start = Math.max(0, idx - 24)
  const before = (start > 0 ? '…' : '') + text.slice(start, idx)
  const match = text.slice(idx, idx + q.length)
  const after = text.slice(idx + q.length)
  return (
    <>
      {before}
      <mark className="chat-message-search-mark">{match}</mark>
      {after}
    </>
  )
}

function SearchResultAvatar({ name, imageUrl }) {
  const color = getAvatarColor(name)
  return (
    <span
      className={`chat-message-search-avatar${imageUrl ? ' chat-message-search-avatar--has-img' : ''}`}
      style={{ ['--avatar-bg']: color.bg, ['--avatar-border']: color.border }}
      aria-hidden="true"
    >
      {imageUrl ? (
        <img src={imageUrl} alt="" className="chat-message-search-avatar-img" />
      ) : (
        (name?.[0] ?? '?').toUpperCase()
      )}
    </span>
  )
}

function ChatMessageSearch({
  open,
  results = [],
  awaiting = false,
  currentUserId = null,
  currentUserName = 'You',
  currentUserAvatarUrl = null,
  correspondentName = '',
  correspondentAvatarUrl = null,
  onSearch,
  onJump,
  onClose,
}) {
  const [query, setQuery] = useState('')
  const inputRef = useRef(null)
  const debounceRef = useRef(null)

  useEffect(() => {
    if (open) {
      inputRef.current?.focus()
    } else {
      setQuery('')
    }
  }, [open])

  useEffect(() => {
    return () => {
      if (debounceRef.current) clearTimeout(debounceRef.current)
    }
  }, [])

  const handleChange = (e) => {
    const value = e.target.value
    setQuery(value)
    if (debounceRef.current) clearTimeout(debounceRef.current)
    const trimmed = value.trim()
    if (!trimmed) {
      onSearch?.('')
      return
    }
    debounceRef.current = setTimeout(() => {
      onSearch?.(trimmed)
    }, SEARCH_DEBOUNCE_MS)
  }

  const handleSubmit = (e) => {
    e.preventDefault()
    if (debounceRef.current) clearTimeout(debounceRef.current)
    const trimmed = query.trim()
    if (trimmed) onSearch?.(trimmed)
  }

  if (!open) return null

  const trimmedQuery = query.trim()
  const showEmpty = !awaiting && trimmedQuery !== '' && results.length === 0

  return (
    <div className="chat-message-search" role="search">
      <form className="chat-message-search-bar" onSubmit={handleSubmit}>
        <span className="chat-message-search-icon" aria-hidden="true">
          <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <circle cx="11" cy="11" r="7" />
            <path d="m21 21-4.3-4.3" />
          </svg>
        </span>
        <input
          ref={inputRef}
          type="text"
          className="chat-message-search-input"
          placeholder="Search in conversation"
          value={query}
          onChange={handleChange}
          aria-label="Search messages in this conversation"
        />
        <button
          type="button"
          className="chat-message-search-close focus-ring"
          onClick={onClose}
          aria-label="Close search"
        >
          <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <path d="M18 6 6 18M6 6l12 12" />
          </svg>
        </button>
      </form>

      {(awaiting || showEmpty || results.length > 0) && (
        <div className="chat-message-search-results" role="listbox" aria-label="Search results">
          {awaiting && (
            <div className="chat-message-search-status chat-message-search-status--loading" role="status">
              <span className="chat-message-search-spinner" aria-hidden="true" />
              <span>Searching…</span>
            </div>
          )}
          {showEmpty && (
            <div className="chat-message-search-status">
              <span className="chat-message-search-empty-icon" aria-hidden="true">
                <svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round">
                  <circle cx="11" cy="11" r="7" />
                  <path d="m21 21-4.3-4.3" />
                </svg>
              </span>
              <span>No messages found for &ldquo;{trimmedQuery}&rdquo;</span>
            </div>
          )}
          {!awaiting && results.length > 0 && (
            <div className="chat-message-search-count" role="presentation">
              {results.length} {results.length === 1 ? 'result' : 'results'}
            </div>
          )}
          {!awaiting &&
            results.map((r) => {
              const mine = currentUserId != null && Number(r.senderId) === Number(currentUserId)
              const authorName = mine ? currentUserName || 'You' : correspondentName || 'Contact'
              const avatarUrl = mine ? currentUserAvatarUrl : correspondentAvatarUrl
              return (
                <button
                  key={r.id}
                  type="button"
                  className="chat-message-search-result focus-ring"
                  role="option"
                  onClick={() => onJump?.(r.id)}
                >
                  <SearchResultAvatar name={authorName} imageUrl={avatarUrl} />
                  <span className="chat-message-search-result-main">
                    <span className="chat-message-search-result-top">
                      <span className={`chat-message-search-result-author${mine ? ' is-mine' : ''}`}>
                        {mine ? 'You' : authorName}
                      </span>
                      <span className="chat-message-search-result-time">
                        {formatMessageSearchDate(r.time)}
                      </span>
                    </span>
                    <span className="chat-message-search-result-text">
                      {highlightSnippet(r.content, trimmedQuery)}
                    </span>
                  </span>
                  <span className="chat-message-search-result-chevron" aria-hidden="true">
                    <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                      <path d="m9 18 6-6-6-6" />
                    </svg>
                  </span>
                </button>
              )
            })}
        </div>
      )}
    </div>
  )
}

export default ChatMessageSearch
