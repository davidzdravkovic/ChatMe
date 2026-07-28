#include "../include/Deserialization/ExtractRequest.h"
#include <iostream>

RequestStruct ExtractRequest::extract(const std::string &json) {
    RequestStruct rStruct;

    auto root = nlohmann::json::parse(json);

    rStruct.sessionID = root.value("SessionId", -1);
    rStruct.action    = root.value("request", std::string{});
    rStruct.data      = root.value("data", nlohmann::json::object());
    rStruct.token     = root.value("token", std::string{});

    setType(rStruct);
    return rStruct;
}

void ExtractRequest::setType(RequestStruct &rStruct) {
    static const std::unordered_map<std::string, RequestType> lookup = {
        {"CREATE_REQUEST",               RequestType::CREATE_REQUEST},
        {"LOGIN_REQUEST",                RequestType::LOGIN_REQUEST},
        {"FETCH_MESSAGES_REQUEST",       RequestType::FETCH_MESSAGES_REQUEST},
        {"MESSAGE_REQUEST",              RequestType::MESSAGE_REQUEST},
        {"FIRST_MESSAGE_REQUEST",        RequestType::FIRST_MESSAGE_REQUEST},
        {"RECENT_CHATROOM_REQUEST",      RequestType::RECENT_CHATROOM_REQUEST},
        {"UPLOAD_PROFILE_PICTURE_REQUEST", RequestType::UPLOAD_PROFILE_PICTURE_REQUEST},
        {"TYPING_REQUEST",               RequestType::TYPING_REQUEST},
        {"SEEN_REQUEST",                 RequestType::SEEN_REQUEST},
        {"UPLOAD_IMAGE_MESSAGE_REQUEST", RequestType::UPLOAD_IMAGE_MESSAGE_REQUEST},
        {"FETCH_IMAGES_FOR_CHAT_REQUEST", RequestType::FETCH_IMAGES_FOR_CHAT_REQUEST},
        {"DISCONNECT_REQUEST",           RequestType::DISCONNECT_REQUEST},
        {"LOGOUT_REQUEST",               RequestType::LOGOUT_REQUEST},
        {"AUTH_REQUEST",                 RequestType::AUTH_REQUEST},
        {"SEARCH_QUERY_REQUEST",         RequestType::SEARCH_QUERY_REQUEST},
        {"REACTION_REQUEST",             RequestType::REACTION_REQUEST},
        {"MESSAGE_SEARCH_REQUEST",       RequestType::MESSAGE_SEARCH_REQUEST},
    };

    auto it = lookup.find(rStruct.action);
    if (it != lookup.end()) {
        rStruct.rType = it->second;
    }
}