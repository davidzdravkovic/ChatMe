import { describe, it, expect, vi, beforeEach } from 'vitest'
import { chatReducer, initialChatState } from './chatReducer'

describe('chatReducer', () => {
  beforeEach(() => {
    // jsdom may omit revokeObjectURL; reducer still calls it when removing a row
    URL.revokeObjectURL = vi.fn()
  })

  it('REMOVE_OPTIMISTIC_BY_TEMP_ID drops the temp row and revokes its preview URL', () => {
    const previewUrl = 'blob:http://localhost/temp-1'
    let state = chatReducer(initialChatState, {
      type: 'OPTIMISTIC_MESSAGE',
      payload: {
        id: 'temp-1',
        temporaryId: 1,
        content: ' ',
        senderId: 99,
        time: new Date().toISOString(),
        mediaId: null,
        localPreviewUrl: previewUrl,
      },
    })
    expect(state.messages).toHaveLength(1)

    state = chatReducer(state, {
      type: 'REMOVE_OPTIMISTIC_BY_TEMP_ID',
      temporaryId: 1,
    })

    expect(state.messages).toHaveLength(0)
    expect(URL.revokeObjectURL).toHaveBeenCalledWith(previewUrl)
  })

  it('REACTION_RESPONSE upserts and removes a user reaction on a message', () => {
    let state = {
      ...initialChatState,
      messages: [
        { id: 10, content: 'hi', senderId: 1, time: '2026-01-01', reactions: [] },
      ],
    }

    state = chatReducer(state, {
      type: 'REACTION_RESPONSE',
      payload: {
        data: [{ messageId: 10, chatroom_id: 5, userId: 2, reaction: '❤️' }],
      },
      activeChatId: 5,
    })
    expect(state.messages[0].reactions).toEqual([{ userId: 2, reaction: '❤️' }])

    state = chatReducer(state, {
      type: 'REACTION_RESPONSE',
      payload: {
        data: [{ messageId: 10, chatroom_id: 5, userId: 2, reaction: '' }],
      },
      activeChatId: 5,
    })
    expect(state.messages[0].reactions).toEqual([])
  })
})
