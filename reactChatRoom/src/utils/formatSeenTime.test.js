import { describe, it, expect } from 'vitest'
import { formatSeenAgo } from './formatSeenTime'

describe('formatSeenTime', () => {
  it('parses postgres timestamptz with short +00 offset', () => {
    const now = new Date('2026-06-15T13:50:00Z')
    expect(formatSeenAgo('2026-06-15 13:48:00+00', now)).toBe('Seen 2m ago')
  })

  it('falls back to plain Seen when timestamp is absent', () => {
    expect(formatSeenAgo(null)).toBe('Seen')
    expect(formatSeenAgo('')).toBe('Seen')
  })
})
