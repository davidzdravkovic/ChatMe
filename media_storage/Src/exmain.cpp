// #ifndef _WIN32_WINNT
// #define _WIN32_WINNT 0x0A00
// #endif
// #ifndef WINVER
// #define WINVER 0x0A00
// #endif
// #include <winsock2.h>
// #include <windows.h>
// #include "../include/httplib.h"
// #include <fstream>
// #include <iostream>
// #include <filesystem>



// int main() {
    
//    httplib::Server server;
//    const std::string TEMP_ROOT = "D:/Media/temp/";
//    const std::string PERM_ROOT = "D:/Media/ProfilePictures/";

//     // ---------- HEALTH CHECK ----------
//     server.Get("/ping", [](const httplib::Request&, httplib::Response& res) {
//         res.set_content("OK", "text/plain");
//         res.status = 200;
//     });

//     // ---------- MEDIA UPLOAD ----------
//    server.Put(R"(/media/temp/([^/]+))",
//     [TEMP_ROOT](const httplib::Request& req, httplib::Response& res) {

//         if (req.body.empty()) {
//             res.status = 400;
//             res.set_content("Empty body", "text/plain");
//             return;
//         }

//         std::filesystem::create_directories(TEMP_ROOT);

//         std::string uploadId = req.matches[1];

//         // sanitize filename for Windows
//         for (char& c : uploadId) {
//             if (c == '/' || c == '\\' || c == ':' || c == '*' ||
//                 c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
//                 c = '_';
//             }
//         }

//         std::string fullPath = TEMP_ROOT + uploadId + ".bin";
//         std::cout << "Saving to: " << fullPath << std::endl;

//         std::ofstream out(fullPath, std::ios::binary);
//         if (!out) {
//             res.status = 500;
//             res.set_content("Failed to open file", "text/plain");
//             return;
//         }

//         out.write(req.body.data(), req.body.size());
//         out.close();

//         res.status = 200;
//         res.set_content("OK", "text/plain");
//     });
//   server.Get(R"(/media/profile/([^/]+))",
// [PERM_ROOT](const httplib::Request& req, httplib::Response& res) {

//     std::string userId = req.matches[1];
//     std::ifstream in(PERM_ROOT + userId + ".bin", std::ios::binary);

//     if (!in) {
//         res.status = 404;
//         return;
//     }

//     std::vector<char> buffer(
//         (std::istreambuf_iterator<char>(in)),
//         std::istreambuf_iterator<char>()
//     );

//     res.set_content(buffer.data(), buffer.size(),
//                     "application/octet-stream");
// });

// server.Post(R"(/media/commit/([^/]+)/([^/]+))",
// [TEMP_ROOT, PERM_ROOT](const httplib::Request& req, httplib::Response& res) {

//     std::filesystem::create_directories(PERM_ROOT);

//     std::string uploadId = req.matches[1];
//     std::string userId   = req.matches[2];

//     std::string tempPath = TEMP_ROOT + uploadId + ".bin";
//     std::string permPath = PERM_ROOT + userId + ".bin";

//     if (!std::filesystem::exists(tempPath)) {
//         res.status = 404;
//         res.set_content("Temp file not found", "text/plain");
//         return;
//     }

//     std::filesystem::rename(tempPath, permPath);

//     res.status = 200;
//     res.set_content("Committed", "text/plain");
// });
// server.Post(R"(/media/message/commit/([^/]+))",
// [TEMP_ROOT](const httplib::Request& req, httplib::Response& res) {

//     const std::string MESSAGE_ROOT = "D:/Media/messages/";
//     std::filesystem::create_directories(MESSAGE_ROOT);

//     std::string mediaId = req.matches[1];

//     std::string tempPath = TEMP_ROOT + mediaId + ".bin";
//     std::string permPath = MESSAGE_ROOT + mediaId + ".bin";

//     if (!std::filesystem::exists(tempPath)) {
//         res.status = 404;
//         res.set_content("Temp file not found", "text/plain");
//         return;
//     }

//     std::filesystem::rename(tempPath, permPath);

//     res.status = 200;
//     res.set_content("Committed", "text/plain");
// });
// server.Get(R"(/media/message/([^/]+))",
// [](const httplib::Request& req, httplib::Response& res) {

//     const std::string MESSAGE_ROOT = "D:/Media/messages/";
//     std::string mediaId = req.matches[1];

//     std::ifstream in(MESSAGE_ROOT + mediaId + ".bin", std::ios::binary);
//     if (!in) {
//         res.status = 404;
//         return;
//     }

//     std::vector<char> buffer(
//         (std::istreambuf_iterator<char>(in)),
//         std::istreambuf_iterator<char>()
//     );

//     res.set_content(buffer.data(), buffer.size(),
//                     "application/octet-stream");
// });


//     std::cout << "Media server running on http://localhost:8081\n";
//     server.listen("0.0.0.0", 8081);
// }
