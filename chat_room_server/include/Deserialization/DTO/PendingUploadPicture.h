#ifndef PENDINGUPLOAD_H
#define PENDINGUPLOAD_H

#include <string>
#include <chrono>

struct PendingUpload {
    std::string uploadId;
    std::string uploadToken;
    int userId;
    std::string uploadUrl;

    std::chrono::steady_clock::time_point expiresAt;
};

#endif
