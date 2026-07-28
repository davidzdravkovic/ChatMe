#include "../include/repositories/MediaDB.h"
#include <stdexcept>

namespace MediaDB {

static MediaState fromDbState(const std::string& state) {
    if (state == "TEMP")  return MediaState::TEMP;
    if (state == "READY") return MediaState::READY;
    throw std::runtime_error("Unknown media state: " + state);
}


int createTempMedia(int ownerUserId, const std::string& mimeType, std::uint64_t fileSizeBytes, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            INSERT INTO media (
                owner_user_id,
                mime_type,
                file_size_bytes,
                state
            )
            VALUES ($1, $2, $3, 'TEMP')
            RETURNING media_id
        )",
        ownerUserId,
        mimeType,
        fileSizeBytes
    );

    if (r.empty()) {
        throw std::runtime_error("Failed to create TEMP media row");
    }

    return r[0]["media_id"].as<int>();
}


bool markMediaReady(int ownerUserId, int mediaId, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            UPDATE media
            SET state = 'READY',
                finalized_at = now()
            WHERE media_id = $1
              AND owner_user_id = $2
              AND state = 'TEMP'
        )",
        mediaId,
        ownerUserId
    );

    return r.affected_rows() >= 1;
}

/**
 * Get media state
 */
std::optional<MediaState> getMediaState(int mediaId, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            SELECT state
            FROM media
            WHERE media_id = $1
        )",
        mediaId
    );

    if (r.empty()) {
        return std::nullopt;
    }

    return fromDbState(r[0]["state"].as<std::string>());
}

/**
 * Attach media as profile picture
 */
bool attachProfilePicture(int ownerUserId, int mediaId, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            UPDATE users
            SET profile_image_media_id = $1,
                profile_image_updated_at = now()
            WHERE user_id = $2
        )",
        mediaId,
        ownerUserId
    );

    return r.affected_rows() == 1;
}

/**
 * Check ownership
 */
bool mediaExistsForUser(int ownerUserId, int mediaId, pqxx::work& tx) {
    pqxx::result r = tx.exec_params(
        R"(
            SELECT 1
            FROM media
            WHERE media_id = $1
              AND owner_user_id = $2
        )",
        mediaId,
        ownerUserId
    );

    return !r.empty();
}

}
