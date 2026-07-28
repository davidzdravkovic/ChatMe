/**
 * Parse server time (ISO or "YYYY-MM-DD HH:MM:SS" or "YYYY-MM-DD HH:MM:SS.ffffff").
 * Postgres `timestamp without time zone` from the VPS is UTC; append Z when missing.
 */
function hasExplicitTimezone(str) {
  return /(?:Z|[+-]\d{2}:?\d{2})$/i.test(str.trim())
}

/** Postgres timestamptz often ends with `+00` — JS needs `+00:00` or `Z`. */
function normalizeIsoTimestamp(str) {
  let s = str.trim()
  if (!s) return s
  if (s.includes(' ') && !s.includes('T')) {
    s = s.replace(' ', 'T')
  }
  if (/[+-]\d{2}$/.test(s) && !/[+-]\d{2}:\d{2}$/.test(s)) {
    s = `${s}:00`
  }
  return s
}

export function parseServerTime(value) {
  if (value == null) return null
  if (value instanceof Date) return value
  const str = String(value).trim()
  if (!str) return null
  const normalized = normalizeIsoTimestamp(str)
  const iso = hasExplicitTimezone(normalized) ? normalized : `${normalized}Z`
  const d = new Date(iso)
  return Number.isNaN(d.getTime()) ? null : d
}
