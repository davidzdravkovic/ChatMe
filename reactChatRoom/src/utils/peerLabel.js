/** Coerce peer username from wire (may be JSON number when name is all digits). */
export function normalizePeerLabel(value) {
  if (value == null) return null
  const t = String(value).trim()
  return t === '' ? null : t
}
