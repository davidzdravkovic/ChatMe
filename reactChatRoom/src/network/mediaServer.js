/**
 * Media server (`media_storage` / httplib) on http://localhost:8081 — see `Src/main.cpp`:
 * - PUT  /media/temp/:uploadId              — legacy temp .bin
 * - POST /media/commit/:id/:userId          — legacy move temp → profile
 * - POST /media/profile/commit/:id/:userId  — body → ProfilePictures/{userId}.bin
 * - GET  /media/profile/:userId             — read profile bytes
 * - POST /media/message/commit/:id — body → messages/{id}.bin (server writes file)
 * - GET  /media/message/:mediaId   — read message attachment
 */

/** Dev: direct to media process. Behind nginx: e.g. VITE_MEDIA_BASE=http://localhost */
const MEDIA_BASE = import.meta.env.VITE_MEDIA_BASE || 'http://localhost:8081'

/** Server-signed path (/media/...) or absolute URL → full HTTP URL for fetch. */
export function resolveMediaHttpUrl(pathOrUrl) {
  if (pathOrUrl == null || pathOrUrl === '') return null
  if (pathOrUrl.startsWith('http://') || pathOrUrl.startsWith('https://')) return pathOrUrl
  const base = MEDIA_BASE.replace(/\/$/, '')
  const path = pathOrUrl.startsWith('/') ? pathOrUrl : `/${pathOrUrl}`
  return `${base}${path}`
}

async function fetchMediaBlobUrl(httpUrl) {
  if (!httpUrl) return null
  try {
    const res = await fetch(httpUrl, { method: 'GET' })
    if (!res.ok) return null
    const blob = await res.blob()
    return URL.createObjectURL(blob)
  } catch {
    return null
  }
}

/**
 * Fetch profile picture for a user. Returns object URL for the image or null on 404/error.
 * Caller should revoke the URL when no longer needed (e.g. cleanup) to avoid leaks.
 * @param {number|string} userId
 * @returns {Promise<string|null>} blob URL or null
 */

//Creating url of received BLOB 
export async function fetchProfileImage(userId, profileUrl = null) {
  if (userId == null) return null
  const httpUrl =
    resolveMediaHttpUrl(profileUrl) ??
    resolveMediaHttpUrl(`/media/profile/${userId}`)
  return fetchMediaBlobUrl(httpUrl)
}

/**
 * Fetch message/chat image. Prefer signed `mediaUrl` from chat server; fallback to legacy bare id.
 * @param {number|string|null} mediaId
 * @param {string|null} [mediaUrl] e.g. /media/message/12?token=...
 */
export async function fetchMessageImage(mediaId, mediaUrl = null) {
  const httpUrl =
    resolveMediaHttpUrl(mediaUrl) ??
    (mediaId != null ? resolveMediaHttpUrl(`/media/message/${mediaId}`) : null)
  return fetchMediaBlobUrl(httpUrl)
}

export function getMediaBase() {
  return MEDIA_BASE
}


/** PUT binary to D:/Media/temp/{uploadId}.bin (matches httplib `Put /media/temp/...`). */
export async function putTempMediaBlob(uploadId, body, mimeType) {
  if (uploadId == null) return false
  try {
    const res = await fetch(`${MEDIA_BASE}/media/temp/${encodeURIComponent(uploadId)}`, {
      method: 'PUT',
      headers: {
        'Content-Type': mimeType || 'application/octet-stream',
      },
      body,
    })
    return res.ok
  } catch {
    return false
  }
}

/** Write chat image bytes (httplib `Post /media/message/commit/:id`). Prefer signed `commitUrl` from INIT. */
export async function postMessageImageCommit(uploadId, body, mimeType, commitUrl = null) {
  if (uploadId == null) return false
  const httpUrl =
    resolveMediaHttpUrl(commitUrl) ??
    resolveMediaHttpUrl(`/media/message/commit/${encodeURIComponent(uploadId)}`)
  if (!httpUrl) return false
  try {
    const res = await fetch(httpUrl, {
      method: 'POST',
      headers: {
        'Content-Type': mimeType || 'application/octet-stream',
      },
      body,
    })
    return res.ok
  } catch {
    return false
  }
}

/** Direct profile upload — prefer signed `commitUrl` from INIT. */
export async function postProfilePictureCommit(uploadId, userId, body, mimeType, commitUrl = null) {
  if (uploadId == null || userId == null) return false
  const httpUrl =
    resolveMediaHttpUrl(commitUrl) ??
    resolveMediaHttpUrl(
      `/media/profile/commit/${encodeURIComponent(uploadId)}/${encodeURIComponent(userId)}`,
    )
  if (!httpUrl) return false
  try {
    const res = await fetch(httpUrl, {
      method: 'POST',
      headers: {
        'Content-Type': mimeType || 'application/octet-stream',
      },
      body,
    })
    return res.ok
  } catch {
    return false
  }
}
