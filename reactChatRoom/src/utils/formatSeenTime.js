import { parseServerTime } from './parseServerTime'

/** Full datetime for seen tooltip: "Jan 27, 2026 at 2:50 PM" */
export function formatSeenTimeFull(value) {
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

/**
 * Relative "Seen …" label from server seen_at (UTC).
 * Falls back to plain "Seen" when timestamp is absent (legacy rows).
 */
export function formatSeenAgo(value, now = new Date()) {
  const d = parseServerTime(value)
  if (!d) return 'Seen'

  const diffMs = now.getTime() - d.getTime()
  if (diffMs < 0) return 'Seen'

  const diffSec = Math.floor(diffMs / 1000)
  if (diffSec < 60) return 'Seen just now'

  const diffMin = Math.floor(diffSec / 60)
  if (diffMin < 60) return `Seen ${diffMin}m ago`

  const diffHr = Math.floor(diffMin / 60)
  if (diffHr < 24) return `Seen ${diffHr}h ago`

  return `Seen ${formatSeenDateLabel(value)}`
}

/** Date-only seen label for reads older than 24h — no clock time. */
function formatSeenDateLabel(value) {
  const d = parseServerTime(value)
  if (!d) return ''
  const today = new Date()
  const sameYear = d.getFullYear() === today.getFullYear()
  return d.toLocaleDateString(undefined, {
    month: 'short',
    day: 'numeric',
    year: sameYear ? undefined : 'numeric',
  })
}
