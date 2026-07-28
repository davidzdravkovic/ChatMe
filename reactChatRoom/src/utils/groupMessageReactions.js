/** Group reactions by emoji for bubble display. */
export function groupMessageReactions(reactions) {
  const map = new Map()
  for (const r of reactions ?? []) {
    if (!r?.reaction) continue
    const key = r.reaction
    if (!map.has(key)) {
      map.set(key, { reaction: key, userIds: [] })
    }
    map.get(key).userIds.push(Number(r.userId))
  }
  return Array.from(map.values())
}
