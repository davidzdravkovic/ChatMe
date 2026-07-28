import { getSessionId } from '../network/wsConnection'

export function createLogStruct(username, password) {
  return {
    SessionId: getSessionId(),
    request: 'LOGIN_REQUEST',
    data: {
      username,
      password,
    },
  }
}

/** Server: explicit user logout (authenticated); clears online registry for this WebSocket session. */
export function createLogoutRequest() {
  return {
    SessionId: getSessionId(),
    request: 'LOGOUT_REQUEST',
    data: {},
  }
}

/** Re-bind business session after reconnect; JWT is attached by sendMessage. Server: AUTH_REQUEST → AUTH_RESPONSE. */
export function createAuthRequest() {
  return {
    SessionId: getSessionId(),
    request: 'AUTH_REQUEST',
    data: {},
  }
}

/** Same envelope as login: SessionId + request + data. Server: CREATE_REQUEST → CREATE_RESPONSE. */
export function createCreateStruct(userName, password, name, email) {
  return {
    SessionId: getSessionId(),
    request: 'CREATE_REQUEST',
    data: {
      userName,
      password,
      name,
      email,
    },
  }
}

// 3️⃣ ChatRetieve — clientFetchEpoch is echoed on FETCH_MESSAGES_RESPONSE so stale fetches after chat switch are ignored
export function createChatRetieve(
  senderUserName,
  receiverUserName,
  limit,
  beforeMessageId = null,
  afterMessageId = null,
  identifier,
  anchorMessageId = null,
) {
  const data = {
    senderUserName,
    receiverUserName,
    limit,
    beforeMessageId,
    afterMessageId,
  }
  if (identifier != null) {
    data.identifier = identifier
  }
  // Search jump: center the initial window on this message id (server treats 0/absent as "no anchor").
  if (anchorMessageId != null && Number(anchorMessageId) > 0) {
    data.anchorMessageId = Number(anchorMessageId)
  }

  return {
    SessionId: getSessionId(),
    request: 'FETCH_MESSAGES_REQUEST',
    data,
  }
}

// 4️⃣ ChatRoomDTO
export function createChatRoomDTO(userID) {
  return {
    SessionId: getSessionId(),
    request: 'RECENT_CHATROOM_REQUEST',
    data: {},
  }
}

// 5️⃣ SendMessageStruct
export function createSendMessageStruct(
  senderUserName,
  receiverUserName,
  content,
  senderId,
  chatroom_id,
  temporaryId,
  replyToMessageId = 0,
) {
  const data = {
    senderUserName,
    receiverUserName,
    content,
    chatroom_id,
    temporaryId,
  }
  if (replyToMessageId > 0)
    data.replyToMessageId = replyToMessageId

  return {
    SessionId: getSessionId(),
    request: 'MESSAGE_REQUEST',
    data,
  }
}

// 6️⃣ TypingRequest
export function createTypingRequest(receiverUser, senderUserName, senderId, chatroom_id, typing) {
  return {
    SessionId: getSessionId(),
    request: 'TYPING_REQUEST',
    data: {
      receiverUser,
      senderUserName,
      chatroom_id,
      typing,
    },
  }
}

// 7️⃣ SeenDTO
export function createSeenDTO(chatroom_id, user_id, last_seen_message_id) {
  return {
    SessionId: getSessionId(),
    request: 'SEEN_REQUEST',
    data: {
      chatroom_id,
      last_seen_message_id,
    },
  }
}

// 8️⃣ FetchDTO
export function createFetchDTO(chatroom_id, user_id, chatIdentifier = null) {
  return {
    SessionId: getSessionId(),
    request: 'FETCH_IMAGES_FOR_CHAT_REQUEST',
    data: {
      chatroom_id,
      user_id,
      chatIdentifier,
    },
  }
}

/** Chat image message — phase 1 (matches DeserializeMediaMessage INIT). */
export function createUploadImageMessageInit(clientId, mimeType, fileSizeBytes) {
  return {
    SessionId: getSessionId(),
    request: 'UPLOAD_IMAGE_MESSAGE_REQUEST',
    data: {
      stage: 'INIT',
      clientId,
      mimeType,
      fileSizeBytes,
    },
  }
}

/** After PUT /media/temp and POST /media/message/commit — creates PENDING_MEDIA message in DB. */
export function createUploadImageMessageCommit(
  clientId,
  uploadId,
  senderUserName,
  receiverUserName,
) {
  return {
    SessionId: getSessionId(),
    request: 'UPLOAD_IMAGE_MESSAGE_REQUEST',
    data: {
      stage: 'COMMIT',
      clientId,
      uploadId,
      senderUserName,
      receiverUserName,
    },
  }
}

/**
 * Marks message READY; server sends MESSAGE_ACK + MESSAGE_RESPONSE.
 * `messageTempId` must match the optimistic row's `temporaryId` (same as INIT `clientId`).
 */
export function createUploadImageMessageFinalize(
  clientId,
  messageTempId,
  uploadId,
  senderUserName,
  receiverUserName,
) {
  return {
    SessionId: getSessionId(),
    request: 'UPLOAD_IMAGE_MESSAGE_REQUEST',
    data: {
      stage: 'FINALIZE',
      clientId,
      messageTempId,
      uploadId,
      senderUserName,
      receiverUserName,
    },
  }
}

/** Profile image: phase 1 — server creates TEMP media row, returns uploadId (media id). */
export function createUploadProfilePictureInit(mimeType, fileSizeBytes) {
  return {
    SessionId: getSessionId(),
    request: 'UPLOAD_PROFILE_PICTURE_REQUEST',
    data: {
      stage: 'INIT',
      mimeType,
      fileSizeBytes,
    },
  }
}

/** Profile image: phase 2 — after bytes are on the media server, mark READY and attach profile. */
export function createUploadProfilePictureCommit(userId, uploadId) {
  return {
    SessionId: getSessionId(),
    request: 'UPLOAD_PROFILE_PICTURE_REQUEST',
    data: {
      stage: 'COMMIT',
      uploadId,
    },
  }
}

export function createFirstMessageDTO(
  senderUserName,
  receiverUserName,
  content,
  senderId,
  temporaryId,
) {
  return {
    SessionId: getSessionId(),
    request: 'FIRST_MESSAGE_REQUEST',
    data: {
      senderUserName,
      receiverUserName,
      content,
      temporaryId,
    },
  }
}

/** Empty `reaction` removes this user's reaction on the message. */
export function createReactionRequest(messageId, chatroom_id, reaction) {
  return {
    SessionId: getSessionId(),
    request: 'REACTION_REQUEST',
    data: {
      messageId,
      chatroom_id,
      reaction: reaction ?? '',
    },
  }
}

export function createSearchQueryDTO(searchedCharacters, searcherQueryId) {
  return {
   SessionId: getSessionId(),
   request: 'SEARCH_QUERY_REQUEST',
   data: {
    searchedCharacters,
    searcherQueryId,
   },
  }
}

/** In-chat message search: server returns MESSAGE_SEARCH_RESPONSE with matching messages for this peer's chat. */
export function createMessageSearchDTO(receiverUserName, searchedText, searchQueryId) {
  return {
    SessionId: getSessionId(),
    request: 'MESSAGE_SEARCH_REQUEST',
    data: {
      receiverUserName,
      searchedText,
      searchQueryId,
    },
  }
}
