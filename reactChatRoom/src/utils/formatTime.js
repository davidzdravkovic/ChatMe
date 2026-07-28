import { parseServerTime } from './parseServerTime'

/**
 * Format time for a message bubble: "2:50 PM", "11:03 AM"
 */
export function formatMessageTime(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  return d.toLocaleTimeString(undefined, {
    hour: 'numeric',
    minute: '2-digit',
    hour12: true,
  })
}

/**
 * Date key for grouping: "2026-01-27"
 */
export function getDateKey(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  const y = d.getFullYear()
  const m = String(d.getMonth() + 1).padStart(2, '0')
  const day = String(d.getDate()).padStart(2, '0')
  return `${y}-${m}-${day}`
}

/**
 * Label for date separator: "Today", "Yesterday", "Monday", or "Jan 27, 2026"
 */
export function formatDateSeparator(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  const today = new Date()
  const key = getDateKey(value)
  const todayKey = getDateKey(today)

  if (key === todayKey) return 'Today'
  const yesterday = new Date(today)
  yesterday.setDate(yesterday.getDate() - 1)
  if (key === getDateKey(yesterday)) return 'Yesterday'
  const daysDiff = Math.round((today - d) / (1000 * 60 * 60 * 24))
  if (daysDiff >= 2 && daysDiff <= 6) {
    return d.toLocaleDateString(undefined, { weekday: 'long' })
  }
  return d.toLocaleDateString(undefined, {
    month: 'short',
    day: 'numeric',
    year: d.getFullYear() !== today.getFullYear() ? 'numeric' : undefined,
  })
}

/**
 * Format date for message search results: "10.02.2026"
 */
export function formatMessageSearchDate(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  const day = String(d.getDate()).padStart(2, '0')
  const m = String(d.getMonth() + 1).padStart(2, '0')
  const y = d.getFullYear()
  return `${day}.${m}.${y}`
}

/**
 * Full datetime for message tooltips: "Jan 27, 2026 at 2:50 PM"
 */
export function formatMessageTimeFull(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  return d.toLocaleString(undefined, {
    month: 'short',
    day: 'numeric',
    year: 'numeric',
    hour: 'numeric',
    minute: '2-digit',
    hour12: true,
  })
}
