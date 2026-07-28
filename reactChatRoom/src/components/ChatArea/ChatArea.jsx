import { useCallback, useEffect, useRef, useState } from 'react'
import './ChatArea.css'
import ChatEmpty from '../ChatEmpty/ChatEmpty'
import ChatConversation from '../ChatConversation/ChatConversation'
import ChatInputBar from '../ChatInputBar/ChatInputBar'
import ChatMessageSearch from '../ChatMessageSearch/ChatMessageSearch'
import AvatarLightbox from '../AvatarLightbox/AvatarLightbox'
import { getAvatarColor } from '../../utils/avatarColor'
import { useChatContext } from '../../context/ChatContext'
import { useLastActiveLabel, isPeerOnline } from '../../hooks/useLastActiveLabel'

function replyPreviewText(message) {
  const text = message?.content != null ? String(message.content).trim() : ''
  if (text) return text.length > 120 ? `${text.slice(0, 120)}…` : text
  const mediaId = message?.mediaId
  if (mediaId != null && mediaId !== 0 && mediaId !== '0') return 'Photo'
  if (message?.localPreviewUrl) return 'Photo'
  return 'Message'
}

function ChatArea({
  activeChat,
  activeChatOnline,
  activeChatLastActiveAt = null,
  canAttachImage,
  onLogout,
  messages,
  typingForActiveChat,
  lastSeenMessageId = null,
  lastSeenAt = null,
  onOpenGallery,
  onOpenChatsList,
  onMessageSent,
  onLoadOlder,
  onLoadNewer,
  onSeen,
  onTyping,
  onChatImageFile,
  onReactToMessage,
  messageSearchOpen = false,
  messageSearchResults = [],
  messageSearchAwaiting = false,
  onOpenMessageSearch,
  onCloseMessageSearch,
  onMessageSearch,
  onJumpToMessage,
  pendingScrollToMessageId = null,
  onConsumePendingScroll,
  anchorViewMessageId = null,
  pendingScrollToBottom = false,
  onConsumePendingScrollToBottom,
  onGoToLatest,
}) {
  const { avatarByUserId, currentUser } = useChatContext()
  const [peerAvatarPreview, setPeerAvatarPreview] = useState(false)
  const [pendingReply, setPendingReply] = useState(null)
  const [headerMenuOpen, setHeaderMenuOpen] = useState(false)
  const optionsRef = useRef(null)
  //From activeChat we are extracting for the current user info plus Ui display 2 paths for rendering empty chat or ChatConversation
  const correspondentName = activeChat?.correspondentName
  const isOnline = isPeerOnline(activeChatOnline)
  const presenceLabel = useLastActiveLabel(activeChatLastActiveAt, isOnline)
  const optionsPresenceLabel = isOnline ? 'Active now' : presenceLabel
  const avatarColor = getAvatarColor(correspondentName)
  const correspondentAvatarUrl = activeChat ? avatarByUserId[activeChat.otherUserId] : null
  const canPreviewPeerAvatar = Boolean(activeChat && correspondentAvatarUrl)

  const clearPendingReply = useCallback(() => setPendingReply(null), [])

  const handleReplyToMessage = useCallback(
    (message) => {
      if (!message?.id || message.temporaryId || Number(message.id) <= 0) return
      const isMine = message.senderId === currentUser?.userId
      setPendingReply({
        messageId: Number(message.id),
        previewContent: replyPreviewText(message),
        previewSenderId: message.senderId,
        previewSenderName: isMine ? 'You' : (correspondentName ?? 'Contact'),
      })
    },
    [currentUser?.userId, correspondentName],
  )

  useEffect(() => {
    setPeerAvatarPreview(false)
    setPendingReply(null)
    setHeaderMenuOpen(false)
  }, [activeChat?.chatRoomId, activeChat?.otherUserId])

  useEffect(() => {
    if (!headerMenuOpen) return undefined
    const onPointer = (e) => {
      if (!optionsRef.current?.contains(e.target)) setHeaderMenuOpen(false)
    }
    const onKey = (e) => {
      if (e.key === 'Escape') setHeaderMenuOpen(false)
    }
    document.addEventListener('mousedown', onPointer)
    document.addEventListener('touchstart', onPointer, { passive: true })
    document.addEventListener('keydown', onKey)
    return () => {
      document.removeEventListener('mousedown', onPointer)
      document.removeEventListener('touchstart', onPointer)
      document.removeEventListener('keydown', onKey)
    }
  }, [headerMenuOpen])

  return (
    <div className="chat-area" role="main" aria-label={activeChat ? `Chat with ${correspondentName}` : 'Select a conversation'}>
      <header className="chat-area-header">
        {onOpenChatsList && (
          <button
            type="button"
            className="chat-area-menu-btn focus-ring"
            onClick={onOpenChatsList}
            aria-label="Open conversation list"
          >
            <svg viewBox="0 0 24 24" width="22" height="22" fill="currentColor" aria-hidden="true">
              <path d="M4 6h16v2H4V6zm0 5h16v2H4v-2zm0 5h16v2H4v-2z" />
            </svg>
          </button>
        )}
        <div className="chat-area-who">
          <div className="chat-area-avatar-wrap">
            {canPreviewPeerAvatar ? (
              <button
                type="button"
                className={`chat-area-avatar chat-area-avatar--btn${correspondentAvatarUrl ? ' chat-area-avatar--has-img' : ''}`}
                style={{
                  ['--avatar-bg']: avatarColor.bg,
                  ['--avatar-border']: avatarColor.border,
                }}
                onClick={() => setPeerAvatarPreview(true)}
                aria-label={`View ${correspondentName ?? 'contact'} profile photo`}
              >
                <img src={correspondentAvatarUrl} alt="" className="chat-area-avatar-img" />
              </button>
            ) : (
              <div
                className={`chat-area-avatar${correspondentAvatarUrl ? ' chat-area-avatar--has-img' : ''}`}
                style={{
                  ['--avatar-bg']: avatarColor.bg,
                  ['--avatar-border']: avatarColor.border,
                }}
                aria-hidden="true"
              >
                {correspondentAvatarUrl ? (
                  <img src={correspondentAvatarUrl} alt="" className="chat-area-avatar-img" />
                ) : (
                  (correspondentName?.[0] ?? '?').toUpperCase()
                )}
              </div>
            )}
          </div>
          <div className="chat-area-info">
            <span className="chat-area-name">{correspondentName ?? 'Select a chat'}</span>
            <span
              className={`chat-area-status ${isOnline ? 'chat-area-status--online' : 'chat-area-status--offline'}`}
              aria-live="polite"
            >
              {activeChat ? presenceLabel : 'Pick a conversation'}
            </span>
          </div>
        </div>
        {activeChat && (
          <div className="chat-area-options" ref={optionsRef}>
            <button
              type="button"
              className={`chat-area-options-btn focus-ring${headerMenuOpen ? ' chat-area-options-btn--active' : ''}`}
              onClick={() => setHeaderMenuOpen((open) => !open)}
              aria-label="Chat options"
              aria-haspopup="menu"
              aria-expanded={headerMenuOpen}
            >
              <svg viewBox="0 0 24 24" width="20" height="20" fill="currentColor" aria-hidden="true">
                <circle cx="12" cy="5" r="1.85" />
                <circle cx="12" cy="12" r="1.85" />
                <circle cx="12" cy="19" r="1.85" />
              </svg>
            </button>
            {headerMenuOpen && (
              <div className="chat-area-options-menu" role="menu">
                <div className="chat-area-options-head">
                  <div
                    className={`chat-area-options-avatar${correspondentAvatarUrl ? ' chat-area-options-avatar--has-img' : ''}`}
                    style={{
                      ['--avatar-bg']: avatarColor.bg,
                      ['--avatar-border']: avatarColor.border,
                    }}
                    aria-hidden="true"
                  >
                    {correspondentAvatarUrl ? (
                      <img src={correspondentAvatarUrl} alt="" className="chat-area-options-avatar-img" />
                    ) : (
                      (correspondentName?.[0] ?? '?').toUpperCase()
                    )}
                    <span
                      className={`chat-area-options-presence ${isOnline ? 'is-online' : 'is-offline'}`}
                    />
                  </div>
                  <div className="chat-area-options-head-info">
                    <span className="chat-area-options-head-name">{correspondentName ?? 'Contact'}</span>
                    <span className="chat-area-options-head-status">
                      {optionsPresenceLabel}
                    </span>
                  </div>
                </div>

                <div className="chat-area-options-divider" role="separator" />

                <button
                  type="button"
                  className="chat-area-options-item focus-ring"
                  role="menuitem"
                  onClick={() => {
                    setHeaderMenuOpen(false)
                    if (messageSearchOpen) onCloseMessageSearch?.()
                    else onOpenMessageSearch?.()
                  }}
                >
                  <span className="chat-area-options-item-icon" aria-hidden="true">
                    <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                      <circle cx="11" cy="11" r="7" />
                      <path d="m21 21-4.3-4.3" />
                    </svg>
                  </span>
                  <span className="chat-area-options-item-text">
                    <span className="chat-area-options-item-label">
                      {messageSearchOpen ? 'Close search' : 'Search messages'}
                    </span>
                    <span className="chat-area-options-item-sub">Find anything in this chat</span>
                  </span>
                </button>

                {canPreviewPeerAvatar && (
                  <button
                    type="button"
                    className="chat-area-options-item focus-ring"
                    role="menuitem"
                    onClick={() => {
                      setHeaderMenuOpen(false)
                      setPeerAvatarPreview(true)
                    }}
                  >
                    <span className="chat-area-options-item-icon" aria-hidden="true">
                      <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                        <rect x="3" y="3" width="18" height="18" rx="3" />
                        <circle cx="8.5" cy="8.5" r="1.6" />
                        <path d="m21 15-5-5L5 21" />
                      </svg>
                    </span>
                    <span className="chat-area-options-item-text">
                      <span className="chat-area-options-item-label">View profile photo</span>
                      <span className="chat-area-options-item-sub">Open full-size avatar</span>
                    </span>
                  </button>
                )}
              </div>
            )}
          </div>
        )}
        <button type="button" className="chat-area-logout focus-ring" onClick={onLogout} aria-label="Log out">
          <svg viewBox="0 0 24 24" width="20" height="20" fill="currentColor" aria-hidden="true">
            <path d="M10 17v-2h4v-6h-4V7l-5 5 5 5Zm9 4H13v-2h6V5h-6V3h6a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2Z" />
          </svg>
        </button>
      </header>

      <div className="chat-area-body">
        {!activeChat && <ChatEmpty />}
         {activeChat && activeChat.initialFetchDone !== false && (
          <>
            <ChatMessageSearch
              open={messageSearchOpen}
              results={messageSearchResults}
              awaiting={messageSearchAwaiting}
              currentUserId={currentUser?.userId ?? null}
              currentUserName={currentUser?.userName ?? 'You'}
              currentUserAvatarUrl={currentUser ? avatarByUserId[currentUser.userId] : null}
              correspondentName={correspondentName}
              correspondentAvatarUrl={correspondentAvatarUrl}
              onSearch={onMessageSearch}
              onJump={onJumpToMessage}
              onClose={onCloseMessageSearch}
            />
            <ChatConversation
              activeChat={activeChat}
              messages={messages}
              typingUser={typingForActiveChat}
              lastSeenMessageId={lastSeenMessageId}
              lastSeenAt={lastSeenAt}
              onLoadOlder={onLoadOlder}
              onLoadNewer={onLoadNewer}
              onSeen={onSeen}
              onReplyToMessage={handleReplyToMessage}
              onReactToMessage={onReactToMessage}
              pendingScrollToMessageId={pendingScrollToMessageId}
              onConsumePendingScroll={onConsumePendingScroll}
              anchorViewMessageId={anchorViewMessageId}
              pendingScrollToBottom={pendingScrollToBottom}
              onConsumePendingScrollToBottom={onConsumePendingScrollToBottom}
              onGoToLatest={onGoToLatest}
            />
            <ChatInputBar
              activeChat={activeChat}
              canAttachImage={canAttachImage}
              onMessageSent={onMessageSent}
              onGalleryClick={onOpenGallery}
              onSendTyping={onTyping}
              onChatImageFile={onChatImageFile}
              replyToMessageId={pendingReply?.messageId ?? 0}
              replyPreviewContent={pendingReply?.previewContent ?? ''}
              replyPreviewSenderId={pendingReply?.previewSenderId ?? 0}
              replyPreviewSenderName={pendingReply?.previewSenderName ?? ''}
              onClearReply={clearPendingReply}
            />
          </>
        )}
        {activeChat && activeChat.initialFetchDone === false && (
          <div className="chat-area-loading" role="status" aria-live="polite" aria-label="Loading chat">
            <span className="chat-area-loading-spinner" aria-hidden="true" />
          </div>
        )}
      </div>
      {peerAvatarPreview && correspondentAvatarUrl && (
        <AvatarLightbox
          imageUrl={correspondentAvatarUrl}
          displayName={correspondentName}
          onClose={() => setPeerAvatarPreview(false)}
        />
      )}
    </div>
  )
}

export default ChatArea
