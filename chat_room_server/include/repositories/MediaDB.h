#ifndef MEDIADB_H
#define MEDIADB_H
#include <string>
#include <optional>
#include <pqxx/pqxx>
#include "./Deserialization/DTO/UploadProfilePicture.h"

namespace MediaDB {

enum class MediaState {
    TEMP,
    READY
};

int createTempMedia(int ownerUserId, const std::string& mimeType, std::uint64_t fileSizeBytes,  pqxx::work& tx);

bool markMediaReady( int ownerUserId, int mediaId, pqxx::work& tx);

std::optional<MediaState> getMediaState( int mediaId, pqxx::work& tx);

bool attachProfilePicture( int ownerUserId, int mediaId, pqxx::work& tx);

bool mediaExistsForUser( int ownerUserId, int mediaId, pqxx::work& tx);

} 

#endif 
