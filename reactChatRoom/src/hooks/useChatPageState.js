import { useReducer, useEffect, useRef, useCallback } from 'react'
import { createChatRoomDTO, createChatRetieve, createFetchDTO, createSeenDTO, createSendMessageStruct, createFirstMessageDTO, createTypingRequest, createSearchQueryDTO, createReactionRequest, createMessageSearchDTO } from '../Dto/dto'
import { sendMessage, subscribeConnection } from '../network/wsConnection'
import { runImageMessageUploadPhases } from '../network/imageMessageUpload'
import { nextClientTemporaryId } from '../utils/nextClientTemporaryId'
import { chatReducer, initialChatState } from '../reducers/chatReducer'
import { ChatSessionEnvironment } from '../controllers/ChatSessionEnvironment'
import { devError, devLog } from '../utils/logger'

const FETCH_LIMIT = 14

const SELF_CHAT_ALERT_MESSAGE = "You can't chat with yourself."

function isOwnUsername(currentUserName, peerName) {
  const me = currentUserName?.trim()
  const peer = peerName?.trim()
  return Boolean(me && peer && me === peer)
}

/**
 * Single hook that owns chat state (reducer), WS subscription, and all chat-specific
 * actions. Keeps "one place" for state updates while keeping ChatPage a thin orchestrator.
 */
export function useChatPageState(currentUser) {
  const [state, dispatch] = useReducer(chatReducer, initialChatState)
  const {
    chats,
    messages,
    activeChat,
    galleryOpen,
    galleryItems,
    typingByChat,
    lastSeenMessageIdByChat,
    seenAtByChat,
    fullScreenImageUrl,
    waitRecentChat,
    chatAlert,
    searchResults,
    searchAwaitingResponse,
    searchHadLatestResponse,
    messageSearchOpen,
    messageSearchResults,
    messageSearchAwaiting,
    pendingScrollToMessageId,
    anchorViewMessageId,
    pendingScrollToBottom,
  } = state

  /** @type {React.MutableRefObject<Record<string, { messages: unknown[] }>>} */
  const optimisticMessagesByPeerRef = useRef({})
  const activeChatRef = useRef(activeChat)
  activeChatRef.current = activeChat
  const typingTimeoutRef = useRef({})
  const pendingGalleryRef = useRef(false)
  const chatSessionEnvRef = useRef(null)
  const bufferOfPendingMessagesRef = useRef({})
  const messageStorageRef = useRef({})
  const temporaryStorageRef = useRef({})
  /** Prefetch for sidebar search (peer not in recent list): peer until FETCH or PEER_USER_NOT_FOUND. */
  const prefetchUnknownPeerRef = useRef(null)
  /** Monotonic id for SEARCH_QUERY_REQUEST; useChatSubscription can compare responses for stale guard. */
  const searchQueryIdRef = useRef(0)
  /** Monotonic id for MESSAGE_SEARCH_REQUEST (in-chat search); stale-response guard. */
  const messageSearchIdRef = useRef(0)

  function isPrefetchingUnknownPeer() {
    const p = prefetchUnknownPeerRef.current
    if (!p) return false
    return activeChatRef.current?.correspondentName !== chatSessionEnvRef.current?.peerUserName
  }

  // After reload the socket may still be connecting; sendMessage drops if not OPEN.
  // Wait until SESSION_INIT (subscribeConnection) so recent chats are actually requested.
  useEffect(() => {
    if (currentUser?.userId == null) return
    const userId = currentUser.userId
    const unsubscribe = subscribeConnection(() => {
      const chatRequest = createChatRoomDTO(userId)
      sendMessage(JSON.stringify(chatRequest))
    })
    return unsubscribe
  }, [currentUser?.userId])

//Needs to run only ONCE in a single chat session
//ActiveChat carries the flag (UI logic + first initial fetch), set to true on chat clicking
//While initialfetch is not done (false) this effect needs to run only ONCE -> Only ONE state update during this phase on activeChat
//Set to false on the first fetch 
  useEffect(() => {
    if (!activeChat) return
    if (activeChat.initialFetchDone === true) return
    const env = chatSessionEnvRef.current
    if (!env) return
    const epoch = env.conversationEpoch
    if (epoch == null) return
    const fetchMessagesRequest = createChatRetieve(
      currentUser.userName,
      activeChat.correspondentName,
      FETCH_LIMIT,
      0,
      0,
      epoch,
    )
    sendMessage(JSON.stringify(fetchMessagesRequest))
  }, [activeChat, currentUser.userName])

  function prevChatRemoveEntry () {

      const prevEnv = chatSessionEnvRef.current
      const prevPeerName = prevEnv?.peerUserName ?? activeChatRef.current?.correspondentName
      if (prevEnv?.state === 'newChat' && prevPeerName && messageStorageRef.current[prevPeerName]) {
      delete messageStorageRef.current[prevPeerName]
    }
    if (prevPeerName && temporaryStorageRef.current[prevPeerName]?.length === 0) {
      delete temporaryStorageRef.current[prevPeerName]
    }
  }



  const handleLoadOlder = useCallback(() => {
    if (isPrefetchingUnknownPeer()) return
    const env = chatSessionEnvRef.current
    if (!env) return
    const oldestId = env.pagination.requestOlder(messages)
    devLog(`the oldest id is ${oldestId}`)
    if (oldestId == null) return
    const chat = activeChatRef.current
    if (!chat) return
    const req = createChatRetieve(
      currentUser.userName,
      chat.correspondentName,
      FETCH_LIMIT,
      oldestId,
      0,
      env.conversationEpoch,
    )
    sendMessage(JSON.stringify(req))
  }, [messages, currentUser.userName])

  const handleLoadNewer = useCallback(() => {
    if (isPrefetchingUnknownPeer()) return
    const env = chatSessionEnvRef.current
    if (!env) return
    const newestId = env.pagination.requestNewer(messages)
    if (newestId == null) return
    const chat = activeChatRef.current
    if (!chat) return
    const req = createChatRetieve(
      currentUser.userName,
      chat.correspondentName,
      FETCH_LIMIT,
      0,
      newestId,
      env.conversationEpoch,
    )
    sendMessage(JSON.stringify(req))
  }, [messages, currentUser.userName])

  const handleSeen = useCallback((messageId) => {
    if (isPrefetchingUnknownPeer()) return
    // No seen until initial fetch has finished for this session
    const chat = activeChatRef.current
    if (chat?.initialFetchDone === false) {
      devLog('Seen action before initial fetch for', chatSessionEnvRef.current?.peerUserName)
      return
    }
    if (chatSessionEnvRef.current?.state === 'newChat') {
      devLog('Seen action during newChat transition for', chatSessionEnvRef.current?.peerUserName)
      return
    }
    const pagination = chatSessionEnvRef.current?.pagination
    if (!chat || !currentUser || messageId == null) return
    if (pagination?.lastSeenSentId === messageId) return
    if (pagination) pagination.lastSeenSentId = messageId
    const req = createSeenDTO(chat.chatRoomId, currentUser.userId, messageId)
    sendMessage(JSON.stringify(req))
  }, [currentUser])

  const handleReaction = useCallback(
    (message, emoji) => {
      const chat = activeChatRef.current
      const uid = currentUser?.userId
      if (!chat?.chatRoomId || uid == null || !message?.id) return
      if (isPrefetchingUnknownPeer()) return
      if (chat.initialFetchDone === false) return
      if (message.temporaryId || Number(message.id) <= 0) return

      const messageId = Number(message.id)
      const mine = (message.reactions ?? []).find((r) => Number(r.userId) === Number(uid))
      const nextReaction = mine?.reaction === emoji ? '' : emoji

      dispatch({
        type: 'REACTION_RESPONSE',
        payload: {
          data: [
            {
              messageId,
              chatroom_id: chat.chatRoomId,
              userId: uid,
              reaction: nextReaction,
            },
          ],
        },
        activeChatId: chat.chatRoomId,
      })

      const req = createReactionRequest(messageId, chat.chatRoomId, nextReaction)
      sendMessage(JSON.stringify(req))
    },
    [currentUser?.userId],
  )

  const handleMessageSent = useCallback((payload) => {
    if(activeChatRef.current == null || currentUser == null) return 
    if (isPrefetchingUnknownPeer()) return
    if (isOwnUsername(payload.currenUsername, payload.correspondentName)) {
      dispatch({
        type: 'SHOW_CHAT_ALERT',
        message: SELF_CHAT_ALERT_MESSAGE,
        variant: 'error',
      })
      return
    }
    if (activeChatRef.current.initialFetchDone === false) {
      devLog('Message sent before initial fetch for', chatSessionEnvRef.current?.peerUserName)
      return
    }

    const sess = chatSessionEnvRef.current
    if (!sess) return

//Always show the optimistic message

      const peerUsername = sess.peerUserName
      if (!temporaryStorageRef.current[peerUsername]) temporaryStorageRef.current[peerUsername] = []
      temporaryStorageRef.current[peerUsername].push(payload)

      const optimistic = {
        id: `temp-${payload.temporaryId}`,
        content: payload.content,
        senderId: payload.senderId,
        time: payload.time,
        temporaryId: payload.temporaryId,
        ...(payload.replyToMessageId > 0
          ? {
              replyToMessageId: payload.replyToMessageId,
              replyPreviewContent: payload.replyPreviewContent ?? '',
              replyPreviewSenderId: payload.replyPreviewSenderId ?? 0,
            }
          : {}),
      }

    //Transition to the last view 
    //All messages after the while the state is inFlight are just storing in temp Storage and released on first FETCH RESPONSE
    if (sess.chatView === `initial`) {
      if (!sess.pagination.inFlight) {
        sess.chatView = `last_view`
        dispatch({ type: 'FETCH_MESSAGES_RESPONSE', payload: { data: [...(messageStorageRef.current[peerUsername] ?? [])] }, mergeMode: 'initial' })
        dispatch({ type: 'OPTIMISTIC_MESSAGE', payload: optimistic })
      } else {
        sess.pendingSeed = true
      }
    }
    else {
      dispatch({ type: 'OPTIMISTIC_MESSAGE', payload: optimistic })
    }

     if (sess.state === 'newChat') { 
      if (sess.subState === `noFirstMessageSent`) {
        devLog('First message sent for new chat with', sess.peerUserName)
        const req = createFirstMessageDTO(
          payload.currenUsername,
          payload.correspondentName,
          payload.content,
          payload.senderId,
          payload.temporaryId,
        )
         sendMessage(JSON.stringify(req))
         sess.subState = `firstMessageSent`
         return
      }
      else if (sess.subState === `firstMessageSent`) {
        const pendingByPeer = bufferOfPendingMessagesRef.current
        const key = payload.correspondentName
        if (!pendingByPeer[key]) pendingByPeer[key] = []
        pendingByPeer[key].push(payload)
      }
      return
    }
    else if (sess.state === 'existingChat') {
      devLog('Message sent for existing chat with', sess.peerUserName, 'chatRoomId:', activeChatRef.current?.chatRoomId, payload.chatRoomId)

   const req  = createSendMessageStruct(
    payload.currenUsername,
    payload.correspondentName,
    payload.content,
    payload.senderId,
    payload.chatRoomId,
    payload.temporaryId,
    payload.replyToMessageId ?? 0,
  )

   sendMessage(JSON.stringify(req))
    }

  }, []) 

  const requestGalleryImages = useCallback(() => {
    if (!activeChat || !currentUser) return
    if (activeChat.chatRoomId == null) return
    const env = chatSessionEnvRef.current
    const epoch = env?.conversationEpoch ?? null
    pendingGalleryRef.current = true
    const req = createFetchDTO(activeChat.chatRoomId, currentUser.userId, epoch)
    sendMessage(JSON.stringify(req))
  }, [activeChat, currentUser])

  const selectChat = useCallback((chat) => {
    //Selecting chat by from recent chat LIST
    if (activeChatRef.current?.correspondentName === chat.correspondentName) return

    prefetchUnknownPeerRef.current = null

    if (isOwnUsername(currentUser?.userName, chat?.correspondentName)) {
      dispatch({
        type: 'SHOW_CHAT_ALERT',
        message: SELF_CHAT_ALERT_MESSAGE,
        variant: 'error',
      })
      return
    }

    //If prev chat was newChat it removes the entry from messageStorageRef
        prevChatRemoveEntry () 
    chatSessionEnvRef.current = new ChatSessionEnvironment(chat.chatRoomId, chat.correspondentName)
    chat.initialFetchDone = false
    dispatch({ type: 'SELECT_ACTIVE_CHAT', payload: chat })
  }, [currentUser?.userName])


  const  selectChatByName = useCallback((correspondentName) => {
    //Selecting chat by SEARCH
    if (activeChatRef.current?.correspondentName === correspondentName) return
    if (isOwnUsername(currentUser?.userName, correspondentName)) {
      dispatch({
        type: 'SHOW_CHAT_ALERT',
        message: SELF_CHAT_ALERT_MESSAGE,
        variant: 'error',
      })
      return
    }
    //If prev chat was newChat it removes the entry from messageStorageRef
    prevChatRemoveEntry()
    const chat = chats.find((c) => c.correspondentName === correspondentName)
    //If the searched user is already in the recent chats
    if (chat) {
      //Creation of the session environment for the lifecycle of one chat 
      selectChat(chat)
      return
    }
    // Not in recent list: keep visible active chat; prefetch with new env + epoch; commit on FETCH_MESSAGES_RESPONSE.
    const peer = String(correspondentName).trim()
    prefetchUnknownPeerRef.current = { peer }
    chatSessionEnvRef.current = new ChatSessionEnvironment(null, peer)
    sendMessage(
      JSON.stringify(
        createChatRetieve(
          currentUser.userName,
          peer,
          FETCH_LIMIT,
          0,
          0,
          chatSessionEnvRef.current.conversationEpoch,
        ),
      ),
    )
  }, [chats, currentUser?.userName, selectChat])


  const closeGallery = useCallback(() => {
    dispatch({ type: 'CLOSE_GALLERY' })
  }, [])

  const setFullscreenImage = useCallback((url) => {
    dispatch({ type: 'SET_FULLSCREEN_IMAGE', payload: url })
  }, [])

  const clearFullscreenImage = useCallback(() => {
    dispatch({ type: 'CLEAR_FULLSCREEN_IMAGE' })
  }, [])

  const setCounterPagination = useCallback(() => {
    dispatch({ type: 'SET_COUNTER_FOR_PAGINATION' })
  }, [])

  const clearChatAlert = useCallback(() => {
    dispatch({ type: 'CLEAR_CHAT_ALERT' })
  }, [])

  const markSearchInputDirty = useCallback(() => {
    dispatch({ type: 'SEARCH_INPUT_DIRTY' })
  }, [])

  const searchQuery = useCallback((searchedCharacters) => {
    dispatch({ type: 'SEARCH_QUERY_SENT' })
    searchQueryIdRef.current += 1
    sendMessage(JSON.stringify(createSearchQueryDTO(searchedCharacters, searchQueryIdRef.current)))
  }, [])

  const clearSearchPanel = useCallback(() => {
    dispatch({ type: 'CLEAR_SEARCH_PANEL' })
  }, [])

  const openMessageSearch = useCallback(() => {
    dispatch({ type: 'OPEN_MESSAGE_SEARCH' })
  }, [])

  const closeMessageSearch = useCallback(() => {
    dispatch({ type: 'CLOSE_MESSAGE_SEARCH' })
  }, [])

  // In-chat message search: server returns matching messages for the active chat only.
  const searchMessagesInChat = useCallback((text) => {
    const chat = activeChatRef.current
    const query = String(text ?? '').trim()
    if (!chat?.correspondentName || !query) {
      dispatch({ type: 'MESSAGE_SEARCH_RESULT', payload: [] })
      return
    }
    messageSearchIdRef.current += 1
    dispatch({ type: 'MESSAGE_SEARCH_SENT' })
    sendMessage(
      JSON.stringify(
        createMessageSearchDTO(chat.correspondentName, query, messageSearchIdRef.current),
      ),
    )
  }, [currentUser.userName])

  // Jump to a searched message: fresh session epoch + anchored initial fetch (window centered on the message),
  // then ChatConversation scrolls it into view (pendingScrollToMessageId).
  const jumpToMessage = useCallback((messageId) => {
    const chat = activeChatRef.current
    const anchorId = Number(messageId)
    if (!chat?.correspondentName || !Number.isFinite(anchorId) || anchorId <= 0) return

    const prevState = chatSessionEnvRef.current?.state
    const env = new ChatSessionEnvironment(chat.chatRoomId, chat.correspondentName)
    // Preserve send-capability for an already-open chat; a fresh env defaults state to undefined.
    if (prevState) env.state = prevState
    env.chatRoomId = chat.chatRoomId
    env.chatView = 'initial'
    chatSessionEnvRef.current = env

    dispatch({ type: 'SET_PENDING_SCROLL', messageId: anchorId })
    dispatch({ type: 'CLOSE_MESSAGE_SEARCH' })

    sendMessage(
      JSON.stringify(
        createChatRetieve(
          currentUser.userName,
          chat.correspondentName,
          FETCH_LIMIT,
          0,
          0,
          env.conversationEpoch,
          anchorId,
        ),
      ),
    )
  }, [currentUser.userName])

  const consumePendingScroll = useCallback(() => {
    dispatch({ type: 'CONSUME_PENDING_SCROLL' })
  }, [])

  const consumePendingScrollToBottom = useCallback(() => {
    dispatch({ type: 'CONSUME_PENDING_SCROLL_TO_BOTTOM' })
  }, [])

  // Leave the anchored (search-jump) view and return to the most recent messages (last_view),
  // re-enabling normal append/prepend pagination. The newest window is always kept in
  // messageStorageRef (the server sends it as the trailing slice on every fetch).
  const goToLatestView = useCallback(() => {
    const env = chatSessionEnvRef.current
    const chat = activeChatRef.current
    if (!env || !chat || chat.initialFetchDone === false) return

    const peer = env.peerUserName
    const stored = [...(messageStorageRef.current[peer] ?? [])]

    if (stored.length > 0) {
      env.chatView = 'last_view'
      dispatch({ type: 'FETCH_MESSAGES_RESPONSE', payload: { data: stored }, mergeMode: 'initial' })
      dispatch({ type: 'EXIT_ANCHOR_VIEW', scrollToBottom: true })
      return
    }

    // Fallback (storage empty): fresh session + plain latest fetch.
    const prevState = env.state
    const fresh = new ChatSessionEnvironment(chat.chatRoomId, chat.correspondentName)
    if (prevState) fresh.state = prevState
    fresh.chatRoomId = chat.chatRoomId
    fresh.chatView = 'initial'
    chatSessionEnvRef.current = fresh
    dispatch({ type: 'EXIT_ANCHOR_VIEW', scrollToBottom: true })
    sendMessage(
      JSON.stringify(
        createChatRetieve(currentUser.userName, chat.correspondentName, FETCH_LIMIT, 0, 0, fresh.conversationEpoch),
      ),
    )
  }, [currentUser.userName])
  


 function onTyping (peerUserName, chatRoomId, typing) {
  if (isPrefetchingUnknownPeer()) return
  if(chatSessionEnvRef.current.state !== `existingChat`) return
    if (!activeChat || !currentUser) return

    const payload = createTypingRequest(
      peerUserName,
      currentUser.userName,
      currentUser.userId,
      chatRoomId,
      typing,
    )
    sendMessage(JSON.stringify(payload))

 }

  const handleChatImageFile = useCallback(
    async (file) => {
      if (!file || !currentUser?.userId) return
      if (isPrefetchingUnknownPeer()) return
      const chat = activeChatRef.current
      if (!chat) return
      if (chatSessionEnvRef.current?.state !== 'existingChat') return

      const env = chatSessionEnvRef.current
      if (!env) return
      const peerUsername = env.peerUserName
      const clientId = nextClientTemporaryId()

      if (!temporaryStorageRef.current[peerUsername]) temporaryStorageRef.current[peerUsername] = []
      temporaryStorageRef.current[peerUsername].push({
        temporaryId: clientId,
        senderId: currentUser.userId,
        content: ' ',
        time: new Date().toISOString(),
      })

      if (env.chatView === 'initial') {
        if (!env.pagination.inFlight) {
          env.chatView = 'last_view'
          dispatch({ type: 'FETCH_MESSAGES_RESPONSE', payload: { data: [...(messageStorageRef.current[peerUsername] ?? [])] }, mergeMode: 'initial' })
        } else {
          env.pendingSeed = true
        }
      }

      try {
        await runImageMessageUploadPhases(file, dispatch, {
          userId: currentUser.userId,
          senderUserName: currentUser.userName,
          receiverUserName: chat.correspondentName,
          clientId,
        })
      } 
      catch (e) {
        devError('Chat image upload failed:', e)
      }
    },
    [currentUser?.userId, currentUser?.userName, dispatch],
  )

  return {
    state: {
      chats,
      messages,
      activeChat,
      galleryOpen,
      galleryItems,
      waitRecentChat,
      typingByChat,
      lastSeenMessageIdByChat,
      seenAtByChat,
      fullScreenImageUrl,
      chatAlert,
      searchResults,
      searchAwaitingResponse,
      searchHadLatestResponse,
      messageSearchOpen,
      messageSearchResults,
      messageSearchAwaiting,
      pendingScrollToMessageId,
      anchorViewMessageId,
      pendingScrollToBottom,
    },
    actions: {
      selectChat,
      selectChatByName,
      handleLoadOlder,
      handleLoadNewer,
      handleSeen,
      handleMessageSent,
      handleReaction,
      requestGalleryImages,
      closeGallery,
      setFullscreenImage,
      clearFullscreenImage,
      setCounterPagination,
      clearChatAlert,
      onTyping,
      handleChatImageFile,
      searchQuery,
      clearSearchPanel,
      markSearchInputDirty,
      openMessageSearch,
      closeMessageSearch,
      searchMessagesInChat,
      jumpToMessage,
      consumePendingScroll,
      consumePendingScrollToBottom,
      goToLatestView,
    },
    subscriptionDeps: {
      dispatch,
      activeChatRef,
      pendingGalleryRef,
      typingTimeoutRef,
      chatSessionEnvRef,
      bufferOfPendingMessagesRef,
      optimisticMessagesByPeerRef,
      messageStorageRef,
      temporaryStorageRef,
      prefetchUnknownPeerRef,
      searchQueryIdRef,
      messageSearchIdRef,
    },
  }
}
