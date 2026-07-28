import { useEffect, useRef } from 'react'
import './ChatConversation.css'
import MessageBubble from '../MessageBubble/MessageBubble'
import { useChatContext } from '../../context/ChatContext'
import { buildConversationItems } from './chatConversationItems'
import { getAvatarColor } from '../../utils/avatarColor'

      const topThreshold = 100
      const bottomThreshold = 250


function ChatConversation({ activeChat, messages, typingUser, lastSeenMessageId = null, lastSeenAt = null, onLoadOlder, onLoadNewer, onSeen, onReplyToMessage, onReactToMessage, pendingScrollToMessageId = null, onConsumePendingScroll, anchorViewMessageId = null, pendingScrollToBottom = false, onConsumePendingScrollToBottom, onGoToLatest }) {
  const { currentUser, avatarByUserId, messageImageByMediaId, loadingMediaIds, setCounterPagination } = useChatContext()
  const anchored = anchorViewMessageId != null

  const showTyping = typingUser?.isTyping && typingUser?.senderUserName && typingUser.senderUserName !== currentUser?.userName
  const correspondentLabel = activeChat?.correspondentName ?? typingUser?.senderUserName ?? ''
  const typingAvatarUrl =
    activeChat?.otherUserId != null ? avatarByUserId[activeChat.otherUserId] : null
  const typingAvatarColor = getAvatarColor(correspondentLabel)
  const scrollRef = useRef(null)
  const triggerScrollPos = useRef(false)

 if (!activeChat) return null

  const items = buildConversationItems({
    messages,
    currentUser,
    correspondentName: activeChat.correspondentName,
    lastSeenMessageId,
    avatarByUserId,
    messageImageByMediaId,
    loadingMediaIds,
    highlightedMessageId: anchorViewMessageId,
  })
    const setSeen = () => {
    if (!activeChat || !messages?.length) return
    const lastMsg = messages[messages.length - 1]
    if (Number(lastMsg.senderId) === Number(currentUser.userId)) return
    const msgId = lastMsg.id
    if (msgId == null || Number(msgId) <= 0) return
    onSeen(msgId)
  }

  //On new message scroll the user to the bottom ##New message append message fetch or message request (if the user is at the threshold)##
  //On typing show the typing indicator and scroll to the bottom if the user is near the bottom
useEffect(() => {

  // A search jump / anchored view owns the scroll position; don't yank to bottom.
  if (pendingScrollToMessageId != null || anchored) return

  const el = scrollRef.current
  if (!el) return

  const atBottom = el.scrollHeight - el.scrollTop - el.clientHeight < bottomThreshold
  if (atBottom) {
    el.scrollTo({ top: el.scrollHeight })
    triggerScrollPos.current = false
    setSeen()
  }
}, [messages, showTyping, pendingScrollToMessageId, anchored])

// Search jump: once the anchored window has rendered, center the target message.
// The highlight + pagination freeze are driven by anchorViewMessageId (reducer state),
// which persists until the user goes to the latest view, jumps again, or changes chat.
useEffect(() => {
  if (pendingScrollToMessageId == null) return
  const el = scrollRef.current
  if (!el) return
  const target = el.querySelector(`[data-message-id="${pendingScrollToMessageId}"]`)
  if (!target) return

  target.scrollIntoView({ block: 'center' })
  triggerScrollPos.current = false

  onConsumePendingScroll?.()
}, [messages, pendingScrollToMessageId, onConsumePendingScroll])

// "Jump to latest": once the most recent window has rendered, snap to the bottom.
useEffect(() => {
  if (!pendingScrollToBottom) return
  const el = scrollRef.current
  if (!el) return
  el.scrollTo({ top: el.scrollHeight })
  triggerScrollPos.current = false
  setSeen()
  onConsumePendingScrollToBottom?.()
}, [messages, pendingScrollToBottom, onConsumePendingScrollToBottom])
   
    
      const onScroll = () => {
      const el = scrollRef.current
      if (!el) return 
      // Anchored search view: keep the focused message put — no append/prepend pagination.
      if (anchored) return
      const notAtTop = el.scrollTop > topThreshold  
      const notAtBottom =el.scrollHeight - el.scrollTop - el.clientHeight > bottomThreshold
      const neutral = (notAtTop) && (notAtBottom)
      if(neutral) {
        triggerScrollPos.current = true;
        return;
      }
     if(triggerScrollPos.current === true) {
      if(!notAtBottom) {
        onLoadNewer?.()
        setCounterPagination?.()
        triggerScrollPos.current = false;
        setSeen()
      }
      else if(!notAtTop) {
        onLoadOlder?.()
        setCounterPagination?.()
        triggerScrollPos.current = false;
      }
     }
    }

 

  return (
    <div className="chat-conversation">
      <div className="chat-conversation-scroll" ref={scrollRef} onScroll={() => {onScroll()}}>
        {items.map((item) => {
          if (item.type === 'date') {
            return (
              <div key={item.key} className="chat-conversation-date" role="separator">
                <span>{item.label}</span>
              </div>
            )
          }
          const { message, isMine, isSeen, isSending, showSeenIndicator, senderUserName, showAvatar, formattedTime, timeFull, hasImage, imageUrl, mediaId, mediaUrl, imageLoading, avatarUrl } = item
          const canInteract =
            !isSending &&
            message?.id != null &&
            !message?.temporaryId &&
            Number(message.id) > 0
          const canReply = canInteract
          const canReact = canInteract
          const replyPreviewAuthor =
            message?.replyPreviewSenderId === currentUser?.userId
              ? 'You'
              : (activeChat.correspondentName ?? 'Contact')
          return (
            <MessageBubble
              key={item.key}
              messageId={message.id}
              highlighted={item.highlighted}
              isMine={isMine}
              isSeen={isSeen}
              isSending={isSending}
              showSeenIndicator={showSeenIndicator}
              seenAt={showSeenIndicator ? lastSeenAt : null}
              senderUserName={senderUserName}
              content={message.content}
              time={formattedTime}
              timeFull={timeFull}
              hasImage={hasImage}
              imageUrl={imageUrl}
              mediaId={mediaId}
              mediaUrl={mediaUrl}
              imageLoading={imageLoading}
              showAvatar={showAvatar}
              avatarUrl={avatarUrl}
              replyToMessageId={message.replyToMessageId ?? 0}
              replyPreviewContent={message.replyPreviewContent ?? ''}
              replyPreviewAuthor={replyPreviewAuthor}
              canReply={canReply}
              onReply={canReply ? () => onReplyToMessage?.(message) : undefined}
              reactions={message.reactions ?? []}
              currentUserId={currentUser?.userId ?? null}
              canReact={canReact}
              onReact={canReact ? (emoji) => onReactToMessage?.(message, emoji) : undefined}
            />
          )
        })}
        {showTyping && (
          <div className="chat-conversation-typing" role="status" aria-live="polite" aria-label={`${typingUser.senderUserName} is typing`}>
            <div
              className={`chat-conversation-typing-avatar${typingAvatarUrl ? ' chat-conversation-typing-avatar--has-img' : ''}`}
              style={{
                ['--avatar-bg']: typingAvatarColor.bg,
                ['--avatar-border']: typingAvatarColor.border,
              }}
              aria-hidden="true"
            >
              {typingAvatarUrl ? (
                <img src={typingAvatarUrl} alt="" className="chat-conversation-typing-avatar-img" />
              ) : (
                (correspondentLabel?.[0] ?? typingUser.senderUserName?.[0] ?? '?').toUpperCase()
              )}
            </div>
            <div className="chat-conversation-typing-bubble">
              <span className="chat-conversation-typing-dots" aria-hidden="true">
                <span className="chat-conversation-typing-dot" />
                <span className="chat-conversation-typing-dot" />
                <span className="chat-conversation-typing-dot" />
              </span>
            </div>
          </div>
        )}
      </div>
      {anchored && (
        <button
          type="button"
          className="chat-conversation-jump-latest focus-ring"
          onClick={() => onGoToLatest?.()}
          aria-label="Jump to latest messages"
        >
          <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
            <path d="M12 5v14" />
            <path d="m19 12-7 7-7-7" />
          </svg>
          <span>Jump to latest</span>
        </button>
      )}
    </div>
  )
}

export default ChatConversation
