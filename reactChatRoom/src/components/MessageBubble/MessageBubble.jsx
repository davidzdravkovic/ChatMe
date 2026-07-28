import { memo, useCallback, useEffect, useId, useLayoutEffect, useRef, useState } from 'react'
import { createPortal } from 'react-dom'
import './MessageBubble.css'
import { useChatContext } from '../../context/ChatContext'
import { groupMessageReactions } from '../../utils/groupMessageReactions'
import { useSeenAgoLabel, seenAtTooltip } from '../../hooks/useSeenAgoLabel'

const QUICK_REACTIONS = ['👍', '❤️', '😂', '😮', '😢']
const LIKE_REACTION = '❤️'
const SWIPE_REPLY_MIN_PX = 56
const SWIPE_LOCK_RATIO = 1.25
const DOUBLE_TAP_MS = 350
const TAP_MOVE_PX = 14
const LIKE_COOLDOWN_MS = 450
const MENU_GAP_PX = 8
const MENU_VIEWPORT_PADDING_PX = 8

const MessageBubble = memo(function MessageBubble({
  messageId = null,
  highlighted = false,
  isMine,
  isSeen,
  isSending = false,
  showSeenIndicator = false,
  seenAt = null,
  senderUserName,
  content,
  time,
  timeFull,
  hasImage,
  imageUrl,
  mediaId,
  mediaUrl,
  imageLoading,
  showAvatar = true,
  avatarUrl,
  replyToMessageId = 0,
  replyPreviewContent = '',
  replyPreviewAuthor = '',
  canReply = false,
  onReply,
  reactions = [],
  currentUserId = null,
  canReact = false,
  onReact,
}) {
  const { loadMessageImage, onMessageImageClick } = useChatContext()
  const seenAgoLabel = useSeenAgoLabel(seenAt, isSeen && showSeenIndicator)
  const seenTitle = seenAtTooltip(seenAt, isSeen)
  const [menuOpen, setMenuOpen] = useState(false)
  const [reactPickerOpen, setReactPickerOpen] = useState(false)
  const [swipeOffset, setSwipeOffset] = useState(0)
  const [menuStyle, setMenuStyle] = useState(null)
  const menuWrapRef = useRef(null)
  const menuPanelRef = useRef(null)
  const menuBtnId = useId()
  const touchStartRef = useRef({ x: 0, y: 0 })
  const lastTapRef = useRef(0)
  const likeCooldownRef = useRef(0)

  const bubbleClass = [
    'message-bubble',
    isMine ? 'message-bubble--mine' : 'message-bubble--theirs',
    showAvatar ? '' : 'message-bubble--consecutive',
    highlighted ? 'message-bubble--highlight' : '',
  ]
    .filter(Boolean)
    .join(' ')

  const hasReply = replyToMessageId > 0
  const groupedReactions = groupMessageReactions(reactions)
  const hasReactions = groupedReactions.length > 0
  const showMenu = (canReply && onReply) || (canReact && onReact)

  const closeMenu = () => {
    setMenuOpen(false)
    setReactPickerOpen(false)
    setMenuStyle(null)
  }

  const positionMenu = useCallback(() => {
    const btn = menuWrapRef.current
    const panel = menuPanelRef.current
    if (!btn || !panel) return

    const rect = btn.getBoundingClientRect()
    const panelRect = panel.getBoundingClientRect()
    const panelWidth = panelRect.width || panel.offsetWidth || 168
    const panelHeight = panelRect.height || panel.offsetHeight || 120
    const spaceBelow = window.innerHeight - rect.bottom - MENU_GAP_PX
    const spaceAbove = rect.top - MENU_GAP_PX
    const openUp = spaceBelow < panelHeight && spaceAbove >= spaceBelow

    let top = openUp ? rect.top - MENU_GAP_PX - panelHeight : rect.bottom + MENU_GAP_PX
    let left = isMine ? rect.right - panelWidth : rect.left

    const maxLeft = window.innerWidth - panelWidth - MENU_VIEWPORT_PADDING_PX
    left = Math.max(MENU_VIEWPORT_PADDING_PX, Math.min(left, maxLeft))
    top = Math.max(
      MENU_VIEWPORT_PADDING_PX,
      Math.min(top, window.innerHeight - panelHeight - MENU_VIEWPORT_PADDING_PX),
    )

    setMenuStyle({ top, left })
  }, [isMine])

  useLayoutEffect(() => {
    if (!menuOpen) return
    positionMenu()
  }, [menuOpen, reactPickerOpen, positionMenu])

  useEffect(() => {
    if (!menuOpen) return
    const onReposition = () => positionMenu()
    window.addEventListener('resize', onReposition)
    window.addEventListener('scroll', onReposition, true)
    return () => {
      window.removeEventListener('resize', onReposition)
      window.removeEventListener('scroll', onReposition, true)
    }
  }, [menuOpen, reactPickerOpen, positionMenu])

  useEffect(() => {
    if (!menuOpen) return
    const onDocPointer = (e) => {
      if (menuWrapRef.current?.contains(e.target)) return
      if (menuPanelRef.current?.contains(e.target)) return
      closeMenu()
    }
    document.addEventListener('mousedown', onDocPointer)
    document.addEventListener('touchstart', onDocPointer, { passive: true })
    return () => {
      document.removeEventListener('mousedown', onDocPointer)
      document.removeEventListener('touchstart', onDocPointer)
    }
  }, [menuOpen])

  const handleReactionChipClick = (emoji, userIds) => {
    if (currentUserId == null) return
    if (userIds.includes(Number(currentUserId))) {
      onReact?.(emoji)
    }
  }

  const handleReactionPick = (emoji) => {
    onReact?.(emoji)
    closeMenu()
  }

  const handleReplyFromMenu = () => {
    closeMenu()
    onReply?.()
  }

  const toggleReactPicker = () => {
    setReactPickerOpen((open) => !open)
  }

  const handleLikeGesture = () => {
    if (!canReact || !onReact) return
    const now = Date.now()
    if (now - likeCooldownRef.current < LIKE_COOLDOWN_MS) return
    likeCooldownRef.current = now
    onReact(LIKE_REACTION)
  }

  const handleBodyDoubleClick = (e) => {
    e.preventDefault()
    e.stopPropagation()
    handleLikeGesture()
  }

  const handleRowTouchStart = (e) => {
    if (menuOpen) return
    const t = e.touches[0]
    if (!t) return
    touchStartRef.current = { x: t.clientX, y: t.clientY }
    setSwipeOffset(0)
  }

  const handleRowTouchMove = (e) => {
    if (menuOpen || !canReply || !onReply) return
    const t = e.touches[0]
    if (!t) return
    const dx = t.clientX - touchStartRef.current.x
    const dy = Math.abs(t.clientY - touchStartRef.current.y)
    const horizontal = Math.abs(dx) > 12 && Math.abs(dx) > dy * SWIPE_LOCK_RATIO
    if (!horizontal) return

    if (isMine && dx < 0) {
      setSwipeOffset(Math.max(dx, -72))
    } else if (!isMine && dx > 0) {
      setSwipeOffset(Math.min(dx, 72))
    }
  }

  const handleRowTouchEnd = (e) => {
    const t = e.changedTouches[0]
    if (!t) {
      setSwipeOffset(0)
      return
    }
    const dx = t.clientX - touchStartRef.current.x
    const dy = Math.abs(t.clientY - touchStartRef.current.y)
    setSwipeOffset(0)

    if (!menuOpen) {
      const horizontal = Math.abs(dx) > dy * SWIPE_LOCK_RATIO
      if (canReply && onReply) {
        if (isMine && dx <= -SWIPE_REPLY_MIN_PX && horizontal) {
          onReply()
          return
        }
        if (!isMine && dx >= SWIPE_REPLY_MIN_PX && horizontal) {
          onReply()
          return
        }
      }

      if (canReact && onReact && Math.abs(dx) <= TAP_MOVE_PX && dy <= TAP_MOVE_PX) {
        const now = Date.now()
        if (now - lastTapRef.current < DOUBLE_TAP_MS) {
          lastTapRef.current = 0
          e.preventDefault()
          handleLikeGesture()
        } else {
          lastTapRef.current = now
        }
      }
    }
  }

  const replyQuote = hasReply ? (
    <div
      className="message-bubble-reply-quote"
      aria-label={`Reply to ${replyPreviewAuthor || 'message'}`}
    >
      <span className="message-bubble-reply-quote-author">{replyPreviewAuthor || 'Message'}</span>
      <span className="message-bubble-reply-quote-text">{replyPreviewContent || 'Message'}</span>
    </div>
  ) : null

  const reactionChips = hasReactions ? (
    <div className="message-bubble-reactions" aria-label="Reactions">
      {groupedReactions.map(({ reaction, userIds }) => {
        const mine = currentUserId != null && userIds.includes(Number(currentUserId))
        return (
          <button
            key={reaction}
            type="button"
            className={`message-bubble-reaction-chip focus-ring${mine ? ' message-bubble-reaction-chip--mine' : ''}`}
            onClick={() => handleReactionChipClick(reaction, userIds)}
            title={userIds.length > 1 ? `${userIds.length} reactions` : 'Reaction'}
            aria-label={`${reaction}${mine ? ', remove your reaction' : ''}`}
          >
            <span className="message-bubble-reaction-emoji" aria-hidden="true">
              {reaction}
            </span>
            {userIds.length > 1 && (
              <span className="message-bubble-reaction-count">{userIds.length}</span>
            )}
          </button>
        )
      })}
    </div>
  ) : null

  const menuList = menuOpen ? (
    <ul
      ref={menuPanelRef}
      className="message-bubble-menu-list message-bubble-menu-list--portal"
      style={
        menuStyle
          ? { top: `${menuStyle.top}px`, left: `${menuStyle.left}px` }
          : { visibility: 'hidden' }
      }
      role="menu"
      aria-labelledby={menuBtnId}
    >
      {canReply && onReply && (
        <li role="none">
          <button
            type="button"
            className="message-bubble-menu-item focus-ring"
            role="menuitem"
            onClick={handleReplyFromMenu}
          >
            <span className="message-bubble-menu-item-icon" aria-hidden="true">
              <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
                <path d="M10 9V5l-7 7 7 7v-4.1c5 0 8.5 1.6 11 5.1-1-5-4-10-11-11z" />
              </svg>
            </span>
            <span className="message-bubble-menu-item-label">Reply</span>
          </button>
        </li>
      )}
      {canReact && onReact && (
        <li role="none">
          <button
            type="button"
            className="message-bubble-menu-item focus-ring"
            role="menuitem"
            onClick={toggleReactPicker}
            aria-expanded={reactPickerOpen}
          >
            <span className="message-bubble-menu-item-icon" aria-hidden="true">
              <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
                <path d="M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8zm3.5-9c.83 0 1.5-.67 1.5-1.5S16.33 9 15.5 9 14 9.67 14 10.5s.67 1.5 1.5 1.5zm-7 0c.83 0 1.5-.67 1.5-1.5S9.33 9 8.5 9 7 9.67 7 10.5 7.67 12 8.5 12zm3.5 6.5c2.33 0 4.31-1.46 5.11-3.5H6.89c.8 2.04 2.78 3.5 5.11 3.5z" />
              </svg>
            </span>
            <span className="message-bubble-menu-item-label">React</span>
            <span className="message-bubble-menu-item-chevron" aria-hidden="true">
              {reactPickerOpen ? '‹' : '›'}
            </span>
          </button>
        </li>
      )}
      {canReact && onReact && reactPickerOpen && (
        <li role="none" className="message-bubble-menu-react-row">
          <div className="message-bubble-quick-react" role="group" aria-label="Quick reactions">
            {QUICK_REACTIONS.map((emoji) => (
              <button
                key={emoji}
                type="button"
                className="message-bubble-quick-react-btn focus-ring"
                onClick={() => handleReactionPick(emoji)}
                aria-label={`React with ${emoji}`}
              >
                {emoji}
              </button>
            ))}
          </div>
        </li>
      )}
      {/* Future menu items: Message info, Unsend */}
    </ul>
  ) : null

  const menuControl = showMenu ? (
    <div className="message-bubble-menu-wrap" ref={menuWrapRef}>
      <button
        type="button"
        id={menuBtnId}
        className="message-bubble-menu-btn focus-ring"
        onClick={() => {
          if (menuOpen) closeMenu()
          else setMenuOpen(true)
        }}
        aria-label="Message options"
        aria-expanded={menuOpen}
        aria-haspopup="menu"
      >
        <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor" aria-hidden="true">
          <circle cx="12" cy="5" r="1.75" />
          <circle cx="12" cy="12" r="1.75" />
          <circle cx="12" cy="19" r="1.75" />
        </svg>
      </button>
      {menuList && createPortal(menuList, document.body)}
    </div>
  ) : null

  const rowStyle =
    swipeOffset !== 0 ? { transform: `translateX(${swipeOffset}px)` } : undefined

  return (
    <div className={bubbleClass} data-message-id={messageId ?? undefined}>
      {!isMine && showAvatar && (
        <div className="message-bubble-avatar">
          {avatarUrl ? (
            <img src={avatarUrl} alt="" className="message-bubble-avatar-img" />
          ) : (
            (senderUserName?.[0] ?? '?').toUpperCase()
          )}
        </div>
      )}
      {!isMine && !showAvatar && <div className="message-bubble-avatar-spacer" aria-hidden="true" />}

      <div className={`message-bubble-stack${isMine ? ' message-bubble-stack--mine' : ''}`}>
        <div
          className={`message-bubble-row${isMine ? ' message-bubble-row--mine' : ''}${swipeOffset !== 0 ? ' message-bubble-row--swiping' : ''}`}
          style={rowStyle}
          onTouchStart={handleRowTouchStart}
          onTouchMove={handleRowTouchMove}
          onTouchEnd={handleRowTouchEnd}
          onTouchCancel={handleRowTouchEnd}
        >
          {canReply && onReply && (
            <span
              className={`message-bubble-swipe-reply-hint${Math.abs(swipeOffset) > 20 ? ' message-bubble-swipe-reply-hint--visible' : ''}`}
              aria-hidden="true"
            >
              <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
                <path d="M10 9V5l-7 7 7 7v-4.1c5 0 8.5 1.6 11 5.1-1-5-4-10-11-11z" />
              </svg>
            </span>
          )}
          <div
            className={`message-bubble-body${hasImage && (!content || !String(content).trim()) ? ' message-bubble-body--image-only' : ''}${hasReply ? ' message-bubble-body--has-reply' : ''}`}
            onDoubleClick={handleBodyDoubleClick}
            title={canReact && onReact ? 'Double-click to like' : undefined}
          >
            {!isMine && showAvatar && senderUserName && (
              <div className="message-bubble-sender">{senderUserName}</div>
            )}
            {replyQuote}

            {hasImage ? (
              <div className="message-bubble-image-wrap">
                <div className="message-bubble-image">
                  {imageUrl ? (
                    <button
                      type="button"
                      className="message-bubble-image-btn focus-ring"
                      onClick={() => onMessageImageClick?.(imageUrl)}
                      aria-label="View image full screen"
                    >
                      <img src={imageUrl} alt="" className="message-bubble-image-img" loading="lazy" />
                      <span className="message-bubble-image-overlay" aria-hidden="true">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                          <path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3" />
                        </svg>
                      </span>
                    </button>
                  ) : imageLoading ? (
                    <div className="message-bubble-image-loading">
                      <span className="message-bubble-image-spinner" aria-hidden="true" />
                      <span className="message-bubble-image-loading-text">Loading…</span>
                    </div>
                  ) : (
                    <button
                      type="button"
                      className="message-bubble-image-load focus-ring"
                      onClick={() => loadMessageImage?.(mediaId, mediaUrl)}
                      aria-label="Load image"
                    >
                      <span className="message-bubble-image-load-icon" aria-hidden="true">
                        <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round">
                          <rect x="3" y="3" width="18" height="18" rx="2" ry="2" />
                          <circle cx="8.5" cy="8.5" r="1.5" />
                          <path d="M21 15l-5-5L5 21" />
                        </svg>
                      </span>
                      <span className="message-bubble-image-load-label">Tap to load image</span>
                    </button>
                  )}
                </div>
                {content && String(content).trim() && (
                  <div className="message-bubble-image-caption message-bubble-text">{content}</div>
                )}
              </div>
            ) : (
              <div className="message-bubble-text">{content}</div>
            )}

            <div className="message-bubble-footer">
              <time className="message-bubble-time" dateTime={timeFull || undefined} title={timeFull || undefined}>
                {time}
              </time>
              {isMine && isSending && (
                <span className="message-bubble-sending" aria-label="Sending" title="Sending…">
                  Sending…
                </span>
              )}
              {isMine && !isSending && showSeenIndicator && (
                <span
                  className={`message-bubble-seen ${isSeen ? 'message-bubble-seen--read' : ''}`}
                  aria-label={isSeen ? seenAgoLabel || 'Seen' : 'Sent'}
                  title={isSeen ? seenTitle : 'Sent'}
                >
                  {isSeen ? (
                    <>
                      <svg className="message-bubble-seen-icon message-bubble-seen-icon--double" viewBox="0 0 24 12" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
                        <path d="M2 6l3 3 6-7" />
                        <path d="M13 6l3 3 6-7" />
                      </svg>
                      <span className="message-bubble-seen-label">{seenAgoLabel || 'Seen'}</span>
                    </>
                  ) : (
                    <svg className="message-bubble-seen-icon message-bubble-seen-icon--single" viewBox="0 0 24 12" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
                      <path d="M2 6l3 3 6-7" />
                    </svg>
                  )}
                </span>
              )}
            </div>
          </div>
          {menuControl}
        </div>
        {reactionChips}
      </div>
    </div>
  )
})

export default MessageBubble
