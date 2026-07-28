import './ChatConnectSkeleton.css'

/**
 * Full-viewport placeholder for the chat shell while the WebSocket (or session) is not ready.
 * Presentational only — parent decides when to show it.
 */
function ChatConnectSkeleton({ label = 'Connecting' }) {
  return (
    <div
      className="chat-connect-skeleton"
      role="status"
      aria-busy="true"
      aria-live="polite"
      aria-label={label}
    >
      <aside className="chat-connect-skeleton__sidebar" aria-hidden="true">
        <div className="chat-connect-skeleton__row chat-connect-skeleton__row--search">
          <span className="chat-connect-skeleton__block chat-connect-skeleton__block--search" />
        </div>
        <ul className="chat-connect-skeleton__list">
          {Array.from({ length: 8 }, (_, i) => (
            <li key={i} className="chat-connect-skeleton__row chat-connect-skeleton__row--list">
              <span className="chat-connect-skeleton__avatar" />
              <span className="chat-connect-skeleton__lines">
                <span className="chat-connect-skeleton__block chat-connect-skeleton__block--line-lg" />
                <span className="chat-connect-skeleton__block chat-connect-skeleton__block--line-sm" />
              </span>
            </li>
          ))}
        </ul>
      </aside>

      <div className="chat-connect-skeleton__divider" aria-hidden="true" />

      <div className="chat-connect-skeleton__main">
        <header className="chat-connect-skeleton__header">
          <span className="chat-connect-skeleton__row chat-connect-skeleton__row--header">
            <span className="chat-connect-skeleton__avatar" />
            <span className="chat-connect-skeleton__lines">
              <span className="chat-connect-skeleton__block chat-connect-skeleton__block--line-lg" />
              <span className="chat-connect-skeleton__block chat-connect-skeleton__block--line-xs" />
            </span>
          </span>
          <span className="chat-connect-skeleton__block chat-connect-skeleton__block--icon" />
        </header>

        <div className="chat-connect-skeleton__messages">
          <span className="chat-connect-skeleton__bubble chat-connect-skeleton__bubble--in" />
          <span className="chat-connect-skeleton__bubble chat-connect-skeleton__bubble--out" />
          <span className="chat-connect-skeleton__bubble chat-connect-skeleton__bubble--in chat-connect-skeleton__bubble--short" />
        </div>

        <div className="chat-connect-skeleton__input" aria-hidden="true">
          <span className="chat-connect-skeleton__block chat-connect-skeleton__block--input" />
        </div>
      </div>
    </div>
  )
}

export default ChatConnectSkeleton
