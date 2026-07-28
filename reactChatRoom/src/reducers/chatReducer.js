import { devLog, devWarn } from '../utils/logger'
import { normalizePeerLabel } from '../utils/peerLabel'
import { mapReplyFromWire } from '../utils/mapReplyFromWire'
import { mapReactionsFromWire } from '../utils/mapReactionsFromWire'

function applyReactionToMessages(messages, { messageId, userId, reaction }) {
  const mid = Number(messageId)
  const uid = Number(userId)
  if (!Number.isFinite(mid) || mid <= 0 || !Number.isFinite(uid) || uid <= 0) {
    return messages
  }

  return messages.map((m) => {
    if (Number(m.id) !== mid) return m
    const next = (m.reactions ?? []).filter((r) => Number(r.userId) !== uid)
    if (reaction) next.push({ userId: uid, reaction: String(reaction) })
    return { ...m, reactions: next }
  })
}

export const initialChatState = {
  chats: [],
  messages: [],
  activeChat: null,
  galleryOpen: false,
  /** Gallery rows from FETCH_IMAGES_FOR_CHAT: { id, mediaUrl }. */
  galleryItems: [],
  waitRecentChat: false,
  typingByChat: {},
  lastSeenMessageIdByChat: {},
  seenAtByChat: {},
  fullScreenImageUrl: null,
  counter: 0,
  /** @type {{ id: number, message: string, variant?: string } | null} */
  chatAlert: null,
  /** Users matching sidebar search (SEARCH_QUERY_RESPONSE); rows { otherUserId, correspondentName }. */
  searchResults: [],
  /** True after a search request is sent until SEARCH_QUERY_RESPONSE applies. */
  searchAwaitingResponse: false,
  /** True only after the latest SEARCH_QUERY_RESPONSE matched the current request (see subscription). */
  searchHadLatestResponse: false,
  /** In-chat message search bar visibility. */
  messageSearchOpen: false,
  /** Matching messages for the active chat (MESSAGE_SEARCH_RESPONSE): rows { id, content, senderId, time }. */
  messageSearchResults: [],
  /** True after a message-search request is sent until its response applies. */
  messageSearchAwaiting: false,
  /** Message id to scroll into view after an anchored (search-jump) fetch lands. */
  pendingScrollToMessageId: null,
  /** Message id of the active anchored (search-jump) view: drives highlight + pagination freeze. */
  anchorViewMessageId: null,
  /** When true, ChatConversation scrolls to the newest message once (used by "Jump to latest"). */
  pendingScrollToBottom: false,
}

/** Non-zero media id from server; null if absent or 0. */

/** Recent list row: last message text + media id when the server sends them. */
function formatChatsFromRecent(data) {
  return data
    .filter((c) => c?.other_username != null && String(c.other_username).trim() !== '')
    .map((c) => ({
      chatRoomId: c.chatroom_id,
      correspondentName: normalizePeerLabel(c.other_username),
      lastMessage: c.content,
      online: c.online === true || c.online === 'true' || c.online === 1 || c.online === '1',
      otherUserId: c.other_userId,
      profileUrl: c.profileUrl ?? null,
      lastActiveAt: c.last_active_at ?? null,
      lastMessageTime: c.time,
      lastMessageHasMedia: c.mediaId ?? null,
    }))
    .filter((c) => c.correspondentName != null)
}

/**
 * Build a recent-list row from live ACK / MESSAGE_RESPONSE.
 * Incoming: peer from Sender. Outgoing (self-sent sync): peer from wire or peerUserName.
 */
function recentChatRowFromAckMessage(
  message,
  { correspondentName, otherUserId, currentUserId, peerUserName, peerUserId },
) {
  const uid = currentUserId != null ? String(currentUserId) : null
  const senderId = message.SenderId != null ? String(message.SenderId) : null
  const incoming = uid != null && senderId != null && senderId !== uid

  const correspondentNameResolved = incoming
    ? normalizePeerLabel(message.Sender)
    : normalizePeerLabel(
        message.ReceiverUserName ??
          message.receiverUserName ??
          peerUserName ??
          correspondentName,
      )

  const receiverIdRaw = message.receiver_id ?? message.receiverId
  const receiverIdNum =
    receiverIdRaw != null && Number(receiverIdRaw) > 0 ? Number(receiverIdRaw) : null

  const otherUserIdResolved = incoming
    ? message.SenderId
    : receiverIdNum ?? peerUserId ?? otherUserId ?? null

  return {
    chatRoomId: message.chatroom_id,
    correspondentName: correspondentNameResolved,
    lastMessage: message.Content,
    lastMessageTime: message.Time,
    lastMessageHasMedia: message.mediaId ?? null,
    online: false,
    otherUserId: otherUserIdResolved,
  }
}

function parseFetchedMessages(data) {
  // Find the index of the marker
  const barrierIndex = data.findIndex(
    (m) => m.endOfInitialSize !== undefined
  )

  // Extract the size (if exists)
  const endOfInitialSize =
    barrierIndex !== -1 ? data[barrierIndex].endOfInitialSize : null

  // Take only the initial messages
  const messages =
    endOfInitialSize != null ? data.slice(0,  barrierIndex) : data
    devLog(`does the payload matches: ${data.length} is equal to ${messages.length}`)
    const messageList = messages.filter((m)=> m.messageId != null)
    

  // Extract last seen (other user's read cursor + timestamp when available)
  let lastSeenIdByOther = null
  let seenAtByOther = null
  for (const item of data) {
    if (item.last_seen_message_id_by_other != null) {
      lastSeenIdByOther = item.last_seen_message_id_by_other
      seenAtByOther = item.seen_at_by_other ?? null
      break
    }
  }

  // Format messages
  const fetchedMessages = messageList.map((m) => ({
    id: m.messageId,
    content: m.Content,
    senderId: m.SenderId,
    time: m.Time,
    mediaId: m.mediaId,
    mediaUrl: m.mediaUrl ?? null,
    chatRoomId: m.chatroom_id,
    ...mapReplyFromWire(m),
    ...mapReactionsFromWire(m),
  }))

  const chatRoomId =
    fetchedMessages.find((m) => m.chatRoomId != null)?.chatRoomId ?? null

  return { fetchedMessages, lastSeenIdByOther, seenAtByOther, chatRoomId }
}

function mergeMessages(prev, fetchedMessages, mergeMode) {
  //Merge the messages
  if (mergeMode === 'prepend') return [...fetchedMessages, ...prev]
  if (mergeMode === 'append') return [...prev, ...fetchedMessages]
  return fetchedMessages
}

export function chatReducer(state, action) {
  switch (action.type) {
    
    case 'RECENT_CHATROOM_RESPONSE':
      return {
        ...state,
        chats: formatChatsFromRecent(action.payload.data),
        waitRecentChat: true,
      }

    case 'FETCH_MESSAGES_RESPONSE': {
      const { fetchedMessages, lastSeenIdByOther, seenAtByOther, chatRoomId } = parseFetchedMessages(
        action.payload.data
      )

      const messages = mergeMessages(state.messages, fetchedMessages, action.mergeMode)
      let lastSeen = state.lastSeenMessageIdByChat
      let seenAt = state.seenAtByChat
      if (lastSeenIdByOther != null && chatRoomId != null) {
        lastSeen = { ...lastSeen, [chatRoomId]: lastSeenIdByOther }
        seenAt = { ...seenAt, [chatRoomId]: seenAtByOther ?? null }
      }
      return { ...state, messages, lastSeenMessageIdByChat: lastSeen, seenAtByChat: seenAt }
    }

    case 'FETCH_IMAGES_FOR_CHAT_RESPONSE': {
      const galleryItems = action.payload.data
        .filter((item) => item.imageId != null && String(item.imageId).trim() !== '')
        .map((item) => ({
          id: item.imageId,
          mediaUrl: item.mediaUrl ?? null,
        }))
      return { ...state, galleryItems, galleryOpen: true }
    }

    case 'TYPING_RESPONSE': {
      const chatRoomId = action.payload.data[0].chatRoomId
      const senderUserName = action.payload.data[0].senderUserName
      const isTyping = action.payload.data[0].isTyping === 'true'
      if (chatRoomId == null) return state
      const next =
        isTyping && senderUserName ? { senderUserName, isTyping } : null
      return {
        ...state,
        typingByChat: { ...state.typingByChat, [chatRoomId]: next },
      }
    }

    case 'TYPING_STALE_CLEAR': {
      const { chatRoomId } = action
      return {
        ...state,
        typingByChat: { ...state.typingByChat, [chatRoomId]: null },
      }
    }

    case 'CHAT_LIST_UPDATE': {
      const message = action.payload.data[0]
      const chatRoomId = message.chatroom_id

      const ackMeta = {
        correspondentName: action.correspondentName,
        otherUserId: action.otherUserId,
        currentUserId: action.currentUserId,
        peerUserName: action.peerUserName,
        peerUserId: action.peerUserId,
      }

      let chats = state.chats
      const updatedChat = chatRoomId != null ? chats.find((c) => c.chatRoomId === chatRoomId) : null

      if (updatedChat) {
        chats = [
          {
            ...updatedChat,
            lastMessage: message.Content,
            lastMessageTime: message.Time,
            lastMessageHasMedia: message.mediaId ?? null,
          },
          ...chats.filter((c) => c.chatRoomId !== chatRoomId),
        ]
      } else if (chatRoomId != null) {
        const row = recentChatRowFromAckMessage(message, ackMeta)
        if (row.correspondentName) {
          chats = [row, ...chats.filter((c) => c.chatRoomId !== chatRoomId)]
        }
      }

      const typingByChat = { ...state.typingByChat, [chatRoomId]: null }

      return { ...state, chats, typingByChat }
    }

    case 'MESSAGE_ACK_RESPONSE': {
      const message = action.payload.data[0]
      const tempId = message.temporaryId
      if (!(tempId > 0)) return state

      const serverMsg = {
        id: message.messageId,
        content: message.Content,
        senderId: message.SenderId,
        time: message.Time,
        mediaId: message.mediaId,
        mediaUrl: message.mediaUrl ?? null,
        ...mapReplyFromWire(message),
        ...mapReactionsFromWire(message),
      }
      const idx = state.messages.findIndex(
        (m) => Number(m.temporaryId) === Number(tempId),
      )
      if (idx < 0) return state

      const messages = [...state.messages]
      const old = messages[idx]
      if (old?.localPreviewUrl) URL.revokeObjectURL(old.localPreviewUrl)
      if (!serverMsg.replyToMessageId && old?.replyToMessageId > 0) {
        serverMsg.replyToMessageId = old.replyToMessageId
        serverMsg.replyPreviewContent = old.replyPreviewContent ?? ''
        serverMsg.replyPreviewSenderId = old.replyPreviewSenderId ?? 0
      }
      messages[idx] = serverMsg
      return { ...state, messages }
    }

    case 'MESSAGE_RESPONSE': {
      const message = action.payload.data[0]
      const peerUserName = normalizePeerLabel(action.peerUserName ?? message.Sender)
      if (peerUserName !== normalizePeerLabel(state.activeChat?.correspondentName)) return state

      const serverMsg = {
        id: message.messageId,
        content: message.Content,
        senderId: message.SenderId,
        time: message.Time,
        mediaId: message.mediaId,
        mediaUrl: message.mediaUrl ?? null,
        ...mapReplyFromWire(message),
        ...mapReactionsFromWire(message),
      }

      const isSelfSent =
        action.currentUserId != null &&
        Number(message.SenderId) === Number(action.currentUserId)

      if (
        state.messages.some(
          (msg) => Number(msg.id) === Number(serverMsg.id),
        )
      ) {
        return state
      }

      if (isSelfSent) {
        const hasPendingTemp = state.messages.some((msg) => Number(msg.temporaryId) > 0)
        if (hasPendingTemp) {
          return state
        }
      }

      return { ...state, messages: [...state.messages, serverMsg] }
    }

    case 'ACTIVE_STATUS_RESPONSE': {
      const data = action.payload.data[0]
      const { userName, status } = data
      const index = state.chats.findIndex((c) => c.correspondentName === userName)
      if (index < 0) return state
      const online = status === 'true'
      const updatedChat = {
        ...state.chats[index],
        online,
        lastActiveAt: online ? null : (data.last_active_at ?? state.chats[index].lastActiveAt ?? null),
      }
      const chats = [
        ...state.chats.slice(0, index),
        updatedChat,
        ...state.chats.slice(index + 1),
      ]
      return { ...state, chats }
    }

    case 'SEEN_RESPONSE': {
      const chatRoomId = action.payload.data[0]?.chatroom_id
      const lastSeenId = action.payload.data[0]?.last_seen_message_id
      const wireSeenAt = action.payload.data[0]?.seen_at
      const seenAt =
        wireSeenAt != null && String(wireSeenAt).trim() !== ''
          ? wireSeenAt
          : lastSeenId != null
            ? new Date().toISOString()
            : null
      if (chatRoomId == null || lastSeenId == null) return state
      return {
        ...state,
        lastSeenMessageIdByChat: {
          ...state.lastSeenMessageIdByChat,
          [chatRoomId]: lastSeenId,
        },
        seenAtByChat: {
          ...state.seenAtByChat,
          [chatRoomId]: seenAt,
        },
      }
    }

    case 'REACTION_RESPONSE': {
      const row = action.payload?.data?.[0]
      if (!row) return state
      const chatRoomId = row.chatroom_id
      if (
        action.activeChatId != null &&
        chatRoomId != null &&
        Number(action.activeChatId) !== Number(chatRoomId)
      ) {
        return state
      }
      return {
        ...state,
        messages: applyReactionToMessages(state.messages, {
          messageId: row.messageId,
          userId: row.userId,
          reaction: row.reaction ?? '',
        }),
      }
    }

    case 'SELECT_ACTIVE_CHAT': {
      return {
        ...state,
        activeChat: action.payload,
        messages: [],
        messageSearchOpen: false,
        messageSearchResults: [],
        messageSearchAwaiting: false,
        pendingScrollToMessageId: null,
        anchorViewMessageId: null,
        pendingScrollToBottom: false,
      }
    }

    case 'UPDATE_ACTIVE_CHAT': {
      return {
        ...state,
        activeChat: action.payload ?? null,
      }
    }


    case 'SEED_FROM_STORAGE': {
      const payloadTempIds = new Set(
        action.payload.filter(m => m.temporaryId != null).map(m => m.temporaryId)
      )
      const extra = state.messages.filter(
        m => m.temporaryId != null && !payloadTempIds.has(m.temporaryId)
      )
      return { ...state, messages: [...action.payload, ...extra] }
    }

    case 'OPTIMISTIC_MESSAGE':
      return { ...state, messages: [...state.messages, action.payload] }

    case 'REMOVE_OPTIMISTIC_BY_TEMP_ID': {
      const tid = Number(action.temporaryId)
      const messages = state.messages.filter((m) => {
        if (Number(m.temporaryId) !== tid) return true
        if (m.localPreviewUrl) URL.revokeObjectURL(m.localPreviewUrl)
        return false
      })
      return { ...state, messages }
    }

    case 'CLOSE_GALLERY':
      return { ...state, galleryOpen: false, galleryItems: [] }

    case 'SET_FULLSCREEN_IMAGE':
      return { ...state, fullScreenImageUrl: action.payload }

    case 'CLEAR_FULLSCREEN_IMAGE':
      return { ...state, fullScreenImageUrl: null }
    
    case `SET_COUNTER_FOR_PAGINATION`:
       return {...state, counter : state.counter + 1}  

    case 'SHOW_CHAT_ALERT':
      return {
        ...state,
        chatAlert: {
          id: Date.now(),
          message: action.message,
          variant: action.variant ?? 'error',
        },
      }

    case 'CLEAR_CHAT_ALERT':
      return { ...state, chatAlert: null }

    case 'SEARCH_INPUT_DIRTY':
      return {
        ...state,
        searchHadLatestResponse: false,
        searchResults: [],
        searchAwaitingResponse: false,
      }

    case 'SEARCH_QUERY_SENT':
      return {
        ...state,
        searchResults: [],
        searchAwaitingResponse: true,
        searchHadLatestResponse: false,
      }

    case 'UPDATE_SEARCH_PANEL': {
      const raw = Array.isArray(action.payload) ? action.payload : []
      const searchResults = raw
        .filter((row) => row?.other_userId != null && row?.other_username != null)
        .map((row) => ({
          otherUserId: row.other_userId,
          correspondentName: String(row.other_username).trim(),
          profileUrl: row.profileUrl ?? null,
        }))
      return {
        ...state,
        searchResults,
        searchAwaitingResponse: false,
        searchHadLatestResponse: true,
      }
    }

    case 'CLEAR_SEARCH_PANEL':
      return {
        ...state,
        searchResults: [],
        searchAwaitingResponse: false,
        searchHadLatestResponse: false,
      }

    case 'OPEN_MESSAGE_SEARCH':
      return { ...state, messageSearchOpen: true }

    case 'CLOSE_MESSAGE_SEARCH':
      return {
        ...state,
        messageSearchOpen: false,
        messageSearchResults: [],
        messageSearchAwaiting: false,
      }

    case 'MESSAGE_SEARCH_SENT':
      return { ...state, messageSearchAwaiting: true }

    case 'MESSAGE_SEARCH_RESULT': {
      const rows = Array.isArray(action.payload) ? action.payload : []
      const messageSearchResults = rows
        .filter((r) => r?.messageId != null)
        .map((r) => ({
          id: r.messageId,
          content: r.Content ?? '',
          senderId: r.SenderId,
          time: r.Time,
        }))
      return { ...state, messageSearchResults, messageSearchAwaiting: false }
    }

    case 'SET_PENDING_SCROLL':
      return {
        ...state,
        pendingScrollToMessageId: action.messageId ?? null,
        anchorViewMessageId: action.messageId ?? null,
        pendingScrollToBottom: false,
      }

    case 'CONSUME_PENDING_SCROLL':
      return { ...state, pendingScrollToMessageId: null }

    case 'EXIT_ANCHOR_VIEW':
      return {
        ...state,
        anchorViewMessageId: null,
        pendingScrollToMessageId: null,
        pendingScrollToBottom: action.scrollToBottom ? true : state.pendingScrollToBottom,
      }

    case 'CONSUME_PENDING_SCROLL_TO_BOTTOM':
      return { ...state, pendingScrollToBottom: false }

    default:
      devWarn('Unhandled action type:', action.type)
      return state
  }
}
