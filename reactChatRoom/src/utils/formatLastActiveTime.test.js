import { describe, it, expect } from 'vitest'
import { formatLastActiveAgo } from './formatLastActiveTime'

describe('formatLastActiveTime', () => {
  it('formats last active ago from postgres timestamptz', () => {
    const now = new Date('2026-06-15T13:50:00Z')
    expect(formatLastActiveAgo('2026-06-15 13:48:00+00', now)).toBe('Last active 2m ago')
  })

  it('returns empty label when timestamp is absent', () => {
    expect(formatLastActiveAgo(null)).toBe('')
    expect(formatLastActiveAgo('')).toBe('')
  })
})
