import { describe, it, expect } from 'vitest'
import { formatMessageTime } from './formatTime'

describe('formatTime', () => {
  it('treats postgres timestamps without timezone as UTC', () => {
    const expected = new Date('2026-06-12T10:00:00Z').toLocaleTimeString(undefined, {
      hour: 'numeric',
      minute: '2-digit',
      hour12: true,
    })
    expect(formatMessageTime('2026-06-12 10:00:00')).toBe(expected)
  })

  it('keeps ISO strings that already include Z', () => {
    const iso = '2026-06-12T10:00:00.000Z'
    const expected = new Date(iso).toLocaleTimeString(undefined, {
      hour: 'numeric',
      minute: '2-digit',
      hour12: true,
    })
    expect(formatMessageTime(iso)).toBe(expected)
  })
})
