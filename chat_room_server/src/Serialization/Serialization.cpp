#include "../include/Serialization/Serialization.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
bool isNumeric(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit(c)) return false;
    return true;
}
} //namespace

bool Serialization::isNumber(const std::string& second) {
    return isNumeric(second);
}

static json buildObjectFromFields(const std::vector<std::pair<std::string, std::string>>& fields) {
    json obj = json::object();
    for (const auto& p : fields) {
        if (isNumeric(p.second))
            obj[p.first] = std::stoi(p.second);
        else
            obj[p.first] = p.second;
    }
    return obj;
}

static std::vector<uint8_t> buildPacket(const json& dataArray, int status, const std::string& responseTypeStr) {
    std::string statusVal = (status == 1) ? "SUCCESS" : "";
    std::string codeVal = (status == 1) ? "200" : "";
    std::string responseStr = responseTypeStr.size() >= 2
        ? responseTypeStr.substr(1, responseTypeStr.size() - 2)
        : responseTypeStr;

    json j;
    j["response"] = responseStr;
    j["status"] = statusVal;
    j["code"] = codeVal;
    j["messages"] = std::to_string(dataArray.size());
    j["data"] = dataArray;

    std::string out = j.dump();
    return std::vector<uint8_t>(out.begin(), out.end());
}

std::vector<uint8_t> Serialization::serialize(std::vector<std::pair<std::string, std::string>> fields, const int& status, const ResponseType& resType) {
    json dataArray = json::array();
    dataArray.push_back(buildObjectFromFields(fields));
    std::vector<uint8_t> finalPacket = buildPacket(dataArray, status, response(resType));
    return finalPacket;
}

std::vector<uint8_t> Serialization::serialize(std::vector<MessageString> mString, const int& status, const ResponseType& resType) {
    json dataArray = json::array();
    for (const auto& ms : mString)
        dataArray.push_back(buildObjectFromFields(ms.pair));
    std::vector<uint8_t> finalPacket = buildPacket(dataArray, status, response(resType));
    return finalPacket;
}

std::vector<uint8_t> Serialization::serialize(std::vector<ChatPreviewString> mString, const int& status, const ResponseType& resType) {
    json dataArray = json::array();
    for (const auto& ms : mString)
        dataArray.push_back(buildObjectFromFields(ms.pair));
    std::vector<uint8_t> finalPacket = buildPacket(dataArray, status, response(resType));
    return finalPacket;
}

std::vector<uint8_t> Serialization::serialize(std::vector<FetchImagesIdString> mString, const int& status, const ResponseType& resType) {
    json dataArray = json::array();
    for (const auto& ms : mString)
        dataArray.push_back(buildObjectFromFields(ms.pair));
    std::vector<uint8_t> finalPacket = buildPacket(dataArray, status, response(resType));
    return finalPacket;
}

std::string Serialization::response(const ResponseType& resType) {
    if (resType == ResponseType::CREATE_RESPONSE) return "\"CREATE_RESPONSE\"";
    if (resType == ResponseType::LOGIN_RESPONSE) return "\"LOGIN_RESPONSE\"";
    if (resType == ResponseType::AUTH_RESPONSE) return "\"AUTH_RESPONSE\"";
    if (resType == ResponseType::FETCH_MESSAGES_RESPONSE) return "\"FETCH_MESSAGES_RESPONSE\"";
    if (resType == ResponseType::MESSAGE_RESPONSE) return "\"MESSAGE_RESPONSE\"";
    if (resType == ResponseType::CHATROOM_ID_RESPONSE) return "\"CHATROOM_ID_RESPONSE\"";
    if (resType == ResponseType::RECENT_CHATROOM_RESPONSE) return "\"RECENT_CHATROOM_RESPONSE\"";
    if (resType == ResponseType::USER_OFFLINE) return "\"USER_OFFLINE\"";
    if (resType == ResponseType::ACTIVE_STATUS_RESPONSE) return "\"ACTIVE_STATUS_RESPONSE\"";
    if (resType == ResponseType::UPLOAD_PROFILE_PICTURE_RESPONSE) return "\"UPLOAD_PROFILE_PICTURE_RESPONSE\"";
    if (resType == ResponseType::UPLOAD_PROFILE_PICTURE_FINISHED_RESPONSE) return "\"UPLOAD_PROFILE_PICTURE_FINISHED_RESPONSE\"";
    if (resType == ResponseType::TYPING_RESPONSE) return "\"TYPING_RESPONSE\"";
    if (resType == ResponseType::SEEN_RESPONSE) return "\"SEEN_RESPONSE\"";
    if (resType == ResponseType::MESSAGE_ACK_RESPONSE) return "\"MESSAGE_ACK_RESPONSE\"";
    if (resType == ResponseType::UPLOAD_IMAGE_MESSAGE_RESPONSE) return "\"UPLOAD_IMAGE_MESSAGE_RESPONSE\"";
    if (resType == ResponseType::UPLOAD_IMAGE_MESSAGE_FINISHED_RESPONSE) return "\"UPLOAD_IMAGE_MESSAGE_FINISHED_RESPONSE\"";
    if (resType == ResponseType::MESSAGE_IMAGE_FINALIZE_RESPONSE) return "\"MESSAGE_IMAGE_FINALIZE_RESPONSE\"";
    if (resType == ResponseType::FETCH_IMAGES_FOR_CHAT_RESPONSE) return "\"FETCH_IMAGES_FOR_CHAT_RESPONSE\"";
    if (resType == ResponseType::PEER_USER_NOT_FOUND_RESPONSE) return "\"PEER_USER_NOT_FOUND_RESPONSE\"";
    if (resType == ResponseType::SEARCH_QUERY_RESPONSE) return "\"SEARCH_QUERY_RESPONSE\"";
    if (resType == ResponseType::REACTION_RESPONSE) return "\"REACTION_RESPONSE\"";
    if (resType == ResponseType::MESSAGE_SEARCH_RESPONSE) return "\"MESSAGE_SEARCH_RESPONSE\"";
    return "\"UNKNOWN\"";
}
