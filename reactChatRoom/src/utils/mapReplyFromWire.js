/** Map optional reply fields from a server message row onto client message shape. */
export function mapReplyFromWire(row) {
  if (!row || typeof row !== 'object') return {}

  const parsed = Number(row.replyToMessageId)
  const replyToMessageId = Number.isFinite(parsed) && parsed > 0 ? parsed : 0
  if (!replyToMessageId) return {}

  const previewSenderParsed = Number(row.replyPreviewSenderId)
  return {
    replyToMessageId,
    replyPreviewContent: row.replyPreviewContent ?? '',
    replyPreviewSenderId:
      Number.isFinite(previewSenderParsed) && previewSenderParsed > 0
        ? previewSenderParsed
        : 0,
  }
}
