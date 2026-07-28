import { useEffect, useState } from 'react'
import { formatSeenAgo, formatSeenTimeFull } from '../utils/formatSeenTime'

/** Live-updating "Seen …" label; refreshes every minute while active. */
export function useSeenAgoLabel(seenAt, active) {
  const [label, setLabel] = useState(() => (active ? formatSeenAgo(seenAt) : ''))

  useEffect(() => {
    if (!active) {
      setLabel('')
      return undefined
    }

    const tick = () => setLabel(formatSeenAgo(seenAt))
    tick()
    const id = setInterval(tick, 60000)
    return () => clearInterval(id)
  }, [seenAt, active])

  return label
}

export function seenAtTooltip(seenAt, isSeen) {
  if (!isSeen) return 'Sent'
  if (seenAt) return formatSeenTimeFull(seenAt)
  return 'Seen'
}
