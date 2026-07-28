import { parseServerTime } from './parseServerTime'

/** Full datetime for last-active tooltip: "Jan 27, 2026 at 2:50 PM" */
export function formatLastActiveTimeFull(value) {
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
 * Relative "Last active …" label from server last_active_at (UTC).
 * Returns empty when timestamp is absent.
 */
export function formatLastActiveAgo(value, now = new Date()) {
  const d = parseServerTime(value)
  if (!d) return ''

  const diffMs = now.getTime() - d.getTime()
  if (diffMs < 0) return ''

  const diffSec = Math.floor(diffMs / 1000)
  if (diffSec < 60) return 'Last active just now'

  const diffMin = Math.floor(diffSec / 60)
  if (diffMin < 60) return `Last active ${diffMin}m ago`

  const diffHr = Math.floor(diffMin / 60)
  if (diffHr < 24) return `Last active ${diffHr}h ago`

  return `Last active ${formatLastActiveTimeFull(value)}`
}
