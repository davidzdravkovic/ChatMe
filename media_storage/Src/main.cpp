#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif
#include <winsock2.h>
#include <windows.h>
#endif
#include "../include/httplib.h"
#include "../include/MediaAuthGate.h"
#include "../include/MediaAccessToken.h"
#include <fstream>
#include <iostream>
#include <filesystem>

static void sanitizePathToken(std::string& id) {
    for (char& c : id) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
}

int main() {
#ifdef _WIN32
   const std::string TEMP_ROOT = "D:/Media/temp/";
   const std::string PERM_ROOT = "D:/Media/ProfilePictures/";
   const std::string MESSAGE_ROOT = "D:/Media/messages/";
#else
   const std::string TEMP_ROOT = "/var/lib/chat-media/temp/";
   const std::string PERM_ROOT = "/var/lib/chat-media/ProfilePictures/";
   const std::string MESSAGE_ROOT = "/var/lib/chat-media/messages/";
#endif

    if (media_auth::mediaJwtSecret().empty() && !media_auth::allowUnsignedRequests()) {
        std::cerr << "media_server: set JWT_SECRET (same as chat_server) or MEDIA_ALLOW_UNSIGNED=1 for local dev\n";
        return 1;
    }
    if (media_auth::allowUnsignedRequests()) {
        std::cerr << "media_server: WARNING MEDIA_ALLOW_UNSIGNED=1 — all /media/* requests allowed without token\n";
    } else {
        std::cerr << "media_server: signed access tokens required on /media/* (?token=...)\n";
    }

   httplib::Server server;

    server.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        media_auth::set_cors(res);
        res.status = 204;
    });

    server.Get("/ping", [](const httplib::Request&, httplib::Response& res) {
        media_auth::set_cors(res);
        res.set_content("OK", "text/plain");
        res.status = 200;
    });

   server.Put(R"(/media/temp/([^/]+))",
    [TEMP_ROOT](const httplib::Request& req, httplib::Response& res) {

        const int uploadId = media_auth::parsePathId(req.matches[1].str());
        if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::WriteTemp, uploadId))
            return;

        if (req.body.empty()) {
            res.status = 400;
            res.set_content("Empty body", "text/plain");
            return;
        }

        std::filesystem::create_directories(TEMP_ROOT);

        std::string uploadIdStr = req.matches[1];
        sanitizePathToken(uploadIdStr);

        std::string fullPath = TEMP_ROOT + uploadIdStr + ".bin";
        std::cout << "Saving to: " << fullPath << std::endl;

        std::ofstream out(fullPath, std::ios::binary);
        if (!out) {
            res.status = 500;
            res.set_content("Failed to open file", "text/plain");
            return;
        }

        out.write(req.body.data(), req.body.size());
        out.close();

        res.status = 200;
        res.set_content("OK", "text/plain");
    });

  server.Get(R"(/media/profile/([^/]+))",
[PERM_ROOT](const httplib::Request& req, httplib::Response& res) {

    const int profileUserId = media_auth::parsePathId(req.matches[1].str());
    if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::ReadProfile, profileUserId))
        return;

    std::string userId = req.matches[1];
    std::ifstream in(PERM_ROOT + userId + ".bin", std::ios::binary);

    if (!in) {
        res.status = 404;
        return;
    }

    std::vector<char> buffer(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    res.set_content(buffer.data(), buffer.size(),
                    "application/octet-stream");
});

server.Post(R"(/media/commit/([^/]+)/([^/]+))",
[TEMP_ROOT, PERM_ROOT](const httplib::Request& req, httplib::Response& res) {

    const int uploadId = media_auth::parsePathId(req.matches[1].str());
    const int userId = media_auth::parsePathId(req.matches[2].str());
    if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::LegacyCommitProfile, uploadId, userId))
        return;

    std::filesystem::create_directories(PERM_ROOT);

    std::string uploadIdStr = req.matches[1];
    std::string userIdStr   = req.matches[2];

    std::string tempPath = TEMP_ROOT + uploadIdStr + ".bin";
    std::string permPath = PERM_ROOT + userIdStr + ".bin";

    if (!std::filesystem::exists(tempPath)) {
        res.status = 404;
        res.set_content("Temp file not found", "text/plain");
        return;
    }

    std::filesystem::rename(tempPath, permPath);

    res.status = 200;
    res.set_content("Committed", "text/plain");
});

server.Post(R"(/media/profile/commit/([^/]+)/([^/]+))",
[PERM_ROOT](const httplib::Request& req, httplib::Response& res) {

    const int uploadId = media_auth::parsePathId(req.matches[1].str());
    const int userId = media_auth::parsePathId(req.matches[2].str());
    if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::CommitProfile, uploadId, userId))
        return;

    if (req.body.empty()) {
        res.status = 400;
        res.set_content("Empty body", "text/plain");
        return;
    }

    std::filesystem::create_directories(PERM_ROOT);

    std::string uploadIdStr = req.matches[1];
    std::string userIdStr   = req.matches[2];
    sanitizePathToken(uploadIdStr);
    sanitizePathToken(userIdStr);

    (void)uploadIdStr;

    std::string permPath = PERM_ROOT + userIdStr + ".bin";
    std::ofstream out(permPath, std::ios::binary);
    if (!out) {
        res.status = 500;
        res.set_content("Failed to open file", "text/plain");
        return;
    }
    out.write(req.body.data(), req.body.size());
    out.close();

    res.status = 200;
    res.set_content("Committed", "text/plain");
});

server.Post(R"(/media/message/commit/([^/]+))",
[MESSAGE_ROOT ](const httplib::Request& req, httplib::Response& res) {

    const int mediaId = media_auth::parsePathId(req.matches[1].str());
    if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::CommitMessage, mediaId))
        return;

    if (req.body.empty()) {
        res.status = 400;
        res.set_content("Empty body", "text/plain");
        return;
    }

    std::filesystem::create_directories(MESSAGE_ROOT);

    std::string mediaIdStr = req.matches[1];
    sanitizePathToken(mediaIdStr);

    std::string permPath = MESSAGE_ROOT + mediaIdStr + ".bin";
    std::ofstream out(permPath, std::ios::binary);
    if (!out) {
        res.status = 500;
        res.set_content("Failed to open file", "text/plain");
        return;
    }
    out.write(req.body.data(), req.body.size());
    out.close();

    res.status = 200;
    res.set_content("Committed", "text/plain");
});

server.Get(R"(/media/message/([^/]+))",
[MESSAGE_ROOT](const httplib::Request& req, httplib::Response& res) {

    const int mediaId = media_auth::parsePathId(req.matches[1].str());
    if (!media_auth::requireMediaAccess(req, res, media_auth::Purpose::ReadMessage, mediaId))
        return;

    std::string mediaIdStr = req.matches[1];

    std::ifstream in(MESSAGE_ROOT + mediaIdStr + ".bin", std::ios::binary);
    if (!in) {
        res.status = 404;
        return;
    }

    std::vector<char> buffer(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    res.set_content(buffer.data(), buffer.size(),
                    "application/octet-stream");
});


#ifdef _WIN32
    const char* bind_host = "0.0.0.0";
    std::cout << "Media server running on http://0.0.0.0:8081\n";
#else
    const char* bind_host = "127.0.0.1";
    std::cout << "Media server running on http://127.0.0.1:8081 (use nginx /media/)\n";
#endif
    server.listen(bind_host, 8081);
}
