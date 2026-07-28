import { useEffect, useState } from 'react'
import { formatLastActiveAgo, formatLastActiveTimeFull } from '../utils/formatLastActiveTime'

function isPeerOnline(value) {
  return value === true || value === 'true' || value === 1 || value === '1'
}

function computePresenceLabel(lastActiveAt, active) {
  if (isPeerOnline(active)) return 'Online'
  if (lastActiveAt) return formatLastActiveAgo(lastActiveAt)
  return 'Offline'
}

/** Live-updating presence label for sidebar + chat header. */
export function useLastActiveLabel(lastActiveAt, active) {
  const online = isPeerOnline(active)
  const [label, setLabel] = useState(() => computePresenceLabel(lastActiveAt, online))

  useEffect(() => {
    if (online) {
      setLabel('Online')
      return undefined
    }
    if (!lastActiveAt) {
      setLabel('Offline')
      return undefined
    }

    const tick = () => setLabel(formatLastActiveAgo(lastActiveAt))
    tick()
    const id = setInterval(tick, 60000)
    return () => clearInterval(id)
  }, [lastActiveAt, online])

  return label
}

export function lastActiveTooltip(lastActiveAt, online) {
  if (isPeerOnline(online)) return 'Online'
  if (lastActiveAt) return formatLastActiveTimeFull(lastActiveAt)
  return 'Offline'
}

export { isPeerOnline }
