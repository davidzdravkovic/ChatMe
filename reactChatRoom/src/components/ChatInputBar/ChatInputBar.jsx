import { useState, useRef, useEffect, useLayoutEffect, useId, useCallback } from 'react'
import { createPortal } from 'react-dom'
import data from '@emoji-mart/data'
import Picker from '@emoji-mart/react'
import './ChatInputBar.css'
import { useChatContext } from '../../context/ChatContext'
import { useIsMobile } from '../../hooks/useIsMobile'
import { nextClientTemporaryId } from '../../utils/nextClientTemporaryId'

const TYPING_DEBOUNCE_MS = 300
const TYPING_IDLE_MS = 2500
const INPUT_MIN_HEIGHT_PX = 44
const INPUT_MAX_HEIGHT_PX = 144
const EMOJI_PICKER_WIDTH_PX = 352
const EMOJI_PICKER_HEIGHT_PX = 435
const EMOJI_PICKER_GAP_PX = 8
const EMOJI_PICKER_VIEWPORT_PADDING_PX = 8

function ChatInputBar({
  activeChat,
  canAttachImage,
  onMessageSent,
  onGalleryClick,
  onSendTyping,
  onChatImageFile,
  replyToMessageId = 0,
  replyPreviewContent = '',
  replyPreviewSenderId = 0,
  replyPreviewSenderName = '',
  onClearReply,
}) {
  const { currentUser } = useChatContext()
  const isMobile = useIsMobile()
  const [text, setText] = useState('')
  const [emojiPickerOpen, setEmojiPickerOpen] = useState(false)
  const [emojiPopoverStyle, setEmojiPopoverStyle] = useState(null)
  const [emojiPickerSession, setEmojiPickerSession] = useState(0)
  //Typing
  const typingDebounceRef = useRef(null)
  const typingIdleRef = useRef(null)
  const lastTypingSentRef = useRef(false)
  const prevActiveChatRef = useRef(null)
  const emojiWrapRef = useRef(null)
  const emojiPopoverRef = useRef(null)
  //Image
  const chatImageInputRef = useRef(null)
  const messageInputRef = useRef(null)
  const chatImageInputId = useId()
  /** Picked image shown in the bar; upload starts only when user presses Send. */
  const [pendingImage, setPendingImage] = useState(null)

  const resizeMessageInput = useCallback(() => {
    const el = messageInputRef.current
    if (!el) return
    el.style.height = 'auto'
    const maxHeight = Number.parseFloat(getComputedStyle(el).maxHeight)
    const cap = Number.isFinite(maxHeight) ? maxHeight : INPUT_MAX_HEIGHT_PX
    const nextHeight = Math.min(Math.max(el.scrollHeight, INPUT_MIN_HEIGHT_PX), cap)
    el.style.height = `${nextHeight}px`
    el.style.overflowY = el.scrollHeight > cap ? 'auto' : 'hidden'
  }, [])

  const clearPendingImage = useCallback(() => {
    setPendingImage((p) => {
      if (p?.url) URL.revokeObjectURL(p.url)
      return null
    })
  }, [])

  useEffect(() => {
    clearPendingImage()
  }, [activeChat?.chatRoomId, clearPendingImage])

  useEffect(() => {
    resizeMessageInput()
  }, [text, resizeMessageInput])

  useEffect(() => {
    const onViewportChange = () => resizeMessageInput()
    window.visualViewport?.addEventListener('resize', onViewportChange)
    window.visualViewport?.addEventListener('scroll', onViewportChange)
    window.addEventListener('orientationchange', onViewportChange)
    return () => {
      window.visualViewport?.removeEventListener('resize', onViewportChange)
      window.visualViewport?.removeEventListener('scroll', onViewportChange)
      window.removeEventListener('orientationchange', onViewportChange)
    }
  }, [resizeMessageInput])

  useEffect(() => {
    if (!(replyToMessageId > 0)) return
    const frame = requestAnimationFrame(() => {
      messageInputRef.current?.focus()
    })
    return () => cancelAnimationFrame(frame)
  }, [replyToMessageId])


  function scheduleTypingStart() {
    if (!activeChat || !currentUser) return
    if (typingIdleRef.current) {
      clearTimeout(typingIdleRef.current)
      typingIdleRef.current = null
    }
    if (typingDebounceRef.current) clearTimeout(typingDebounceRef.current)
    typingDebounceRef.current = setTimeout(() => {
      typingDebounceRef.current = null
      onSendTyping(activeChat.correspondentName, activeChat?.chatRoomId, true)
          lastTypingSentRef.current = true
      typingIdleRef.current = setTimeout(() => {
        typingIdleRef.current = null
        onSendTyping(activeChat.correspondentName, activeChat?.chatRoomId, false)
            lastTypingSentRef.current = false
      }, TYPING_IDLE_MS)
    }, TYPING_DEBOUNCE_MS)
  }

  function cancelTypingAndSendStop() {
    if (typingDebounceRef.current) {
      clearTimeout(typingDebounceRef.current)
      typingDebounceRef.current = null
    }
    if (typingIdleRef.current) {
      clearTimeout(typingIdleRef.current)
      typingIdleRef.current = null
    }
    if (lastTypingSentRef.current) {
      onSendTyping(activeChat.correspondentName, activeChat?.chatRoomId, false)
          lastTypingSentRef.current = false
    }
  }

  // When switching chat: send typing false for the previous chat and clear timers.
  // Cleanup uses `activeForThisEffect` (chat when this run committed), not `prev` from first mount.
  useEffect(() => {
    const activeForThisEffect = activeChat
    const prev = prevActiveChatRef.current
    prevActiveChatRef.current = activeChat
    if (prev && activeChat && prev.chatRoomId !== activeChat.chatRoomId && lastTypingSentRef.current) {
      onSendTyping(prev.correspondentName, prev.chatRoomId, false)
      lastTypingSentRef.current = false
    }
    return () => {
      if (typingDebounceRef.current) clearTimeout(typingDebounceRef.current)
      if (typingIdleRef.current) clearTimeout(typingIdleRef.current)
      if (lastTypingSentRef.current && activeForThisEffect?.correspondentName != null) {
        onSendTyping(
          activeForThisEffect.correspondentName,
          activeForThisEffect.chatRoomId,
          false,
        )
        lastTypingSentRef.current = false
      }
      typingDebounceRef.current = null
      typingIdleRef.current = null
    }
  }, [activeChat?.chatRoomId])

  useEffect(() => {
    if (isMobile) {
      setEmojiPickerOpen(false)
      setEmojiPopoverStyle(null)
    }
  }, [isMobile])

  const positionEmojiPopover = useCallback(() => {
    const btn = emojiWrapRef.current?.querySelector('.chat-input-bar-btn--emoji')
    if (!btn) return

    const rect = btn.getBoundingClientRect()
    const panelHeight = emojiPopoverRef.current?.offsetHeight || EMOJI_PICKER_HEIGHT_PX
    const panelWidth = emojiPopoverRef.current?.offsetWidth || EMOJI_PICKER_WIDTH_PX
    const spaceAbove = rect.top - EMOJI_PICKER_GAP_PX
    const spaceBelow = window.innerHeight - rect.bottom - EMOJI_PICKER_GAP_PX
    const openUp = spaceAbove >= panelHeight || spaceAbove >= spaceBelow

    let top = openUp
      ? rect.top - EMOJI_PICKER_GAP_PX - panelHeight
      : rect.bottom + EMOJI_PICKER_GAP_PX
    let left = rect.left

    const maxLeft = window.innerWidth - panelWidth - EMOJI_PICKER_VIEWPORT_PADDING_PX
    left = Math.max(EMOJI_PICKER_VIEWPORT_PADDING_PX, Math.min(left, maxLeft))
    top = Math.max(
      EMOJI_PICKER_VIEWPORT_PADDING_PX,
      Math.min(top, window.innerHeight - panelHeight - EMOJI_PICKER_VIEWPORT_PADDING_PX),
    )

    setEmojiPopoverStyle({ top, left })
  }, [])

  useLayoutEffect(() => {
    if (!emojiPickerOpen || isMobile) return
    positionEmojiPopover()
    const frame = requestAnimationFrame(() => {
      positionEmojiPopover()
    })
    return () => cancelAnimationFrame(frame)
  }, [emojiPickerOpen, isMobile, emojiPickerSession, positionEmojiPopover])

  useEffect(() => {
    if (!emojiPickerOpen || isMobile) return
    const onReposition = () => positionEmojiPopover()
    window.addEventListener('resize', onReposition)
    window.addEventListener('scroll', onReposition, true)
    return () => {
      window.removeEventListener('resize', onReposition)
      window.removeEventListener('scroll', onReposition, true)
    }
  }, [emojiPickerOpen, isMobile, positionEmojiPopover])

  useEffect(() => {
    if (!emojiPickerOpen || isMobile) return
    const onPointerDown = (e) => {
      if (emojiWrapRef.current?.contains(e.target)) return
      if (emojiPopoverRef.current?.contains(e.target)) return
      setEmojiPickerOpen(false)
      setEmojiPopoverStyle(null)
    }
    document.addEventListener('pointerdown', onPointerDown, true)
    return () => document.removeEventListener('pointerdown', onPointerDown, true)
  }, [emojiPickerOpen, isMobile])

  function handleEmojiSelect(emoji) {
    const ch = emoji?.native ?? ''
    if (ch) setText((t) => t + ch)
    setEmojiPickerOpen(false)
    setEmojiPopoverStyle(null)
    resizeMessageInput()
  }

  const emojiPopover =
    !isMobile && emojiPickerOpen
      ? createPortal(
          <div
            ref={emojiPopoverRef}
            className="chat-input-bar-emoji-popover chat-input-bar-emoji-popover--portal"
            role="dialog"
            aria-label="Emoji picker"
            style={emojiPopoverStyle ?? undefined}
          >
            <Picker
              key={emojiPickerSession}
              data={data}
              theme="dark"
              icons="solid"
              onEmojiSelect={handleEmojiSelect}
              previewPosition="none"
              searchPosition="sticky"
              navPosition="top"
              skinTonePosition="search"
              dynamicWidth={false}
              perLine={9}
              emojiSize={24}
              emojiButtonSize={36}
              maxFrequentRows={1}
            />
          </div>,
          document.body,
        )
      : null

  function handleSubmit(e) {
    e.preventDefault()
    if (!activeChat || !currentUser) return

    //If we have text and image write this is going to sent just the image on the submit the text is staying in the input bar
    if (pendingImage?.file) {
      const file = pendingImage.file
      clearPendingImage()
      onChatImageFile?.(file)
      cancelTypingAndSendStop()
    return
    }

    const content = text.trim()
    if (!content) return

    const now = new Date()
    const temporaryId = nextClientTemporaryId()
    const timeStr = now.toISOString ? now.toISOString() : now.toLocaleString()

    // Optimistic: add bubble immediately with "Sending" (works even when server is offline)
    onMessageSent?.({
      currenUsername : currentUser.userName,
      correspondentName: activeChat.correspondentName,
      content,
      senderId: currentUser.userId,
      time: timeStr,
      temporaryId,
      chatRoomId: activeChat.chatRoomId,
      ...(replyToMessageId > 0
        ? { replyToMessageId, replyPreviewContent, replyPreviewSenderId }
        : {}),
    })
    setText('')
    onClearReply?.()
    cancelTypingAndSendStop()
  }

  function handleInputKeyDown(e) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault()
      e.currentTarget.form?.requestSubmit()
    }
  }

  //Preparing the text content 
  function handleInputChange(e) {
    setText(e.target.value)
    scheduleTypingStart()
    resizeMessageInput()
  }

  function handleBlur() {
    cancelTypingAndSendStop()
  }

  function setPendingImageFile(file) {
    if (!file) return
    setPendingImage((p) => {
      if (p?.url) URL.revokeObjectURL(p.url)
      return { file, url: URL.createObjectURL(file) }
    })
  }

  function handleChatImageChange(e) {
    setPendingImageFile(e.target.files?.[0])
    e.target.value = ''
  }

  function handlePaste(e) {
    if (!canAttachImage) return
    const imageItem = [...e.clipboardData.items].find((item) => item.type.startsWith('image/'))
    if (!imageItem) return
    e.preventDefault()
    setPendingImageFile(imageItem.getAsFile())
  }

  return (
    <form
      className="chat-input-bar"
      role="form"
      aria-label="Compose message"
      onSubmit={handleSubmit}
    >
      {replyToMessageId > 0 && (
        <div className="chat-input-bar-pending-reply" aria-live="polite">
          <div className="chat-input-bar-pending-reply-inner">
            <div className="chat-input-bar-pending-reply-bar" aria-hidden="true" />
            <div className="chat-input-bar-pending-reply-meta">
              <span className="chat-input-bar-pending-reply-label">
                Replying to {replyPreviewSenderName || 'message'}
              </span>
              <span className="chat-input-bar-pending-reply-text">{replyPreviewContent}</span>
            </div>
            <button
              type="button"
              className="chat-input-bar-pending-remove focus-ring"
              aria-label="Cancel reply"
              onClick={() => onClearReply?.()}
            >
              ×
            </button>
          </div>
        </div>
      )}
      {pendingImage && (
        <div className="chat-input-bar-pending-image" aria-live="polite">
          <div className="chat-input-bar-pending-image-inner">
            <img
              src={pendingImage.url}
              alt=""
              className="chat-input-bar-pending-thumb"
            />
            <div className="chat-input-bar-pending-meta">
              <span className="chat-input-bar-pending-label">Image ready to send</span>
              <span className="chat-input-bar-pending-hint">Press send to upload</span>
            </div>
            <button
              type="button"
              className="chat-input-bar-pending-remove focus-ring"
              aria-label="Remove image"
              onClick={clearPendingImage}
            >
              x
            </button>
          </div>
        </div>
      )}
      <div className="chat-input-bar-row">
        <div className="chat-input-bar-composer">
          <input
            ref={chatImageInputRef}
            id={chatImageInputId}
            type="file"
            className="chat-input-bar-file-input"
            accept="image/jpeg,image/png,image/webp,image/gif,image/*"
            tabIndex={-1}
            aria-hidden="true"
            onChange={handleChatImageChange}
          />
          <button
            type="button"
            className="chat-input-bar-btn chat-input-bar-btn--secondary chat-input-bar-btn--attach chat-input-bar-btn--leading focus-ring"
            aria-label="Attach image"
            disabled={!canAttachImage}
            title={canAttachImage ? 'Attach image' : 'Open an existing chat to send images'}
            onClick={() => canAttachImage && chatImageInputRef.current?.click()}
          >
            <svg viewBox="0 0 24 24" className="chat-input-bar-icon chat-input-bar-icon--camera" aria-hidden="true">
              <path
                fill="currentColor"
                d="M9.4 4.8h5.2l.9 1.8h2.7A2.7 2.7 0 0 1 20.8 9.3v8.4A2.7 2.7 0 0 1 18.1 20.4H5.9A2.7 2.7 0 0 1 3.2 17.7V9.3A2.7 2.7 0 0 1 5.9 6.6h2.7l.8-1.8ZM12 16.1a3.6 3.6 0 1 0 0-7.2 3.6 3.6 0 0 0 0 7.2Z"
              />
            </svg>
          </button>
          <textarea
            ref={messageInputRef}
            className="chat-input-bar-input focus-ring"
            rows={1}
            placeholder="Message…"
            aria-label="Message input"
            autoComplete="off"
            enterKeyHint="enter"
            value={text}
            onChange={handleInputChange}
            onInput={resizeMessageInput}
            onKeyDown={handleInputKeyDown}
            onBlur={handleBlur}
            onPaste={handlePaste}
          />
          <div className="chat-input-bar-toolbar">
            {!isMobile && (
              <div className="chat-input-bar-emoji-wrap" ref={emojiWrapRef}>
                <button
                  type="button"
                  className="chat-input-bar-btn chat-input-bar-btn--secondary chat-input-bar-btn--emoji focus-ring"
                  aria-label="Add emoji"
                  aria-expanded={emojiPickerOpen}
                  aria-haspopup="dialog"
                  onClick={() => {
                    setEmojiPickerOpen((open) => {
                      const next = !open
                      if (next) setEmojiPickerSession((s) => s + 1)
                      else setEmojiPopoverStyle(null)
                      return next
                    })
                  }}
                >
                  <svg viewBox="0 0 24 24" className="chat-input-bar-icon" aria-hidden="true">
                    <path fill="currentColor" d="M12 22a10 10 0 1 1 0-20 10 10 0 0 1 0 20Zm0-18a8 8 0 1 0 0 16 8 8 0 0 0 0-16Zm-3 8a1 1 0 1 1 0-2 1 1 0 0 1 0 2Zm6 0a1 1 0 1 1 0-2 1 1 0 0 1 0 2Zm-3 6c-2.2 0-4.1-1.2-5.1-3l1.7-1c.7 1.2 2 2 3.4 2s2.7-.8 3.4-2l1.7 1c-1 1.8-2.9 3-5.1 3Z" />
                  </svg>
                </button>
              </div>
            )}
            <button
              type="button"
              className="chat-input-bar-btn chat-input-bar-btn--secondary chat-input-bar-btn--gallery focus-ring"
              aria-label="Gallery"
              onClick={onGalleryClick}
            >
              <svg viewBox="0 0 24 24" className="chat-input-bar-icon chat-input-bar-icon--gallery" aria-hidden="true">
                <path
                  fill="currentColor"
                  d="M5 4.8h14A2.2 2.2 0 0 1 21.2 7v10A2.2 2.2 0 0 1 19 19.2H5A2.2 2.2 0 0 1 2.8 17V7A2.2 2.2 0 0 1 5 4.8Zm0 1.6a.6.6 0 0 0-.6.6v9.8l4.1-4.4a1.2 1.2 0 0 1 1.7 0l2.4 2.5 4.5-5.2a1.2 1.2 0 0 1 1.8 0L19.2 17V7a.6.6 0 0 0-.6-.6H5Zm3.2 2.4a1.8 1.8 0 1 1 0 3.6 1.8 1.8 0 0 1 0-3.6Z"
                />
              </svg>
            </button>
            <button
              type="submit"
              className="chat-input-bar-btn chat-input-bar-btn--send focus-ring"
              aria-label={pendingImage ? 'Send image' : 'Send message'}
            >
              <svg viewBox="0 0 24 24" className="chat-input-bar-icon chat-input-bar-icon--send" aria-hidden="true">
                <path fill="currentColor" d="m3.4 20.6 18.2-8.6L3.4 3.4v5.8l11.2 1.5L3.4 12.2v8.4Z" />
              </svg>
            </button>
          </div>
        </div>
      </div>
      {emojiPopover}
    </form>
  )
}

export default ChatInputBar
