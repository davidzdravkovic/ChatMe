#pragma once
#include <string>

enum class MessageStatus {
    READY,
    PENDING_MEDIA,
    FAILED
};

inline std::string toDbStatus(MessageStatus s) {
    switch (s) {
        case MessageStatus::READY:         return "READY";
        case MessageStatus::PENDING_MEDIA: return "PENDING_MEDIA";
        case MessageStatus::FAILED:        return "FAILED";
    }
    return "READY";
}