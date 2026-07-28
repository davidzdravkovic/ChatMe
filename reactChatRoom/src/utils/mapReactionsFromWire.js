/** Map server `reactions` field onto client message shape (always returns an array). */
export function mapReactionsFromWire(row) {
  if (!row || typeof row !== 'object') return { reactions: [] }

  let raw = row.reactions
  if (raw == null || raw === '') return { reactions: [] }

  let parsed = raw
  if (typeof raw === 'string') {
    try {
      parsed = JSON.parse(raw)
    } catch {
      return { reactions: [] }
    }
  }

  if (!Array.isArray(parsed)) return { reactions: [] }

  const reactions = parsed
    .map((r) => ({
      userId: Number(r.userId),
      reaction: String(r.reaction ?? ''),
    }))
    .filter((r) => Number.isFinite(r.userId) && r.userId > 0 && r.reaction)

  return { reactions }
}
