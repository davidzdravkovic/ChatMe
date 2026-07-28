#ifndef DTOFIELDSVALUES_H
#define DTOFIELDSVALUES_H
#include "AllDtoFiles.h"
#include "DtoFields.h"
#include <array>
#include <string_view>






template<>
struct dtoFields<ChatRetieve> {

    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"senderUserName"},   &ChatRetieve::senderUserName },
        std::pair{ std::string_view{"receiverUserName"}, &ChatRetieve::receiverUserName }
        
    };
       static constexpr auto ints = std::array{
              std::pair{ std::string_view{"limit"}, &ChatRetieve::limit},
              std::pair{ std::string_view{"identifier"}, &ChatRetieve::identifier}
    };
    static constexpr auto optional = std::array{
            std::pair{ std::string_view{"beforeMessageId"}, &ChatRetieve::beforeMessageId},
            std::pair{ std::string_view{"afterMessageId"}, &ChatRetieve::afterMessageId},
            std::pair{ std::string_view{"anchorMessageId"}, &ChatRetieve::anchorMessageId}
        };
    };
 


// ==================================================
// CreateStruct
// ==================================================
template<>
struct dtoFields<CreateStruct> {

    static constexpr auto strings = std::array{
        std::pair{ "userName", &CreateStruct::userName },
        std::pair{ "password", &CreateStruct::password },
        std::pair{ "name",     &CreateStruct::name },
        std::pair{ "email",    &CreateStruct::email }
    };
};


template<>
struct dtoFields<LogStruct> {

    static constexpr auto strings = std::array{
        std::pair{ "username", &LogStruct::userName },
        std::pair{ "password", &LogStruct::password }
    };
};



template<>
struct dtoFields<ChatRoomDTO> {

    // userId: from JWT in Handler (bindTrustedUser), not from client data
    static constexpr auto ints = std::array<std::pair<std::string_view, int ChatRoomDTO::*>, 0>{};
};


template<>
struct dtoFields<SendMessageStruct> {

    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"senderUserName"}, &SendMessageStruct::senderUserName },
        std::pair{ std::string_view{"receiverUserName"},   &SendMessageStruct::receiverUserName },
        std::pair{ std::string_view{"content"},        &SendMessageStruct::content }
    };

    // senderId: from JWT in Handler (bindTrustedUser), not from client data
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"chatroom_id"}, &SendMessageStruct::chatRoomId },
        std::pair{ std::string_view{"temporaryId"}, &SendMessageStruct::temporaryId },
        std::pair{ std::string_view{"replyToMessageId"}, &SendMessageStruct::replyToMessageId },
    };
};
template<>
struct dtoFields<SendFirstMessageStruct> {

    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"senderUserName"}, &SendFirstMessageStruct::senderUserName },
        std::pair{ std::string_view{"receiverUserName"},   &SendFirstMessageStruct::receiverUserName },
        std::pair{ std::string_view{"content"},        &SendFirstMessageStruct::content }
    };

    // senderId: from JWT in Handler
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"temporaryId"}, &SendFirstMessageStruct::temporaryId }
    };
};



template<>
struct dtoFields<TypingRequest> {

    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"senderUserName"},&TypingRequest::senderUserName},
        std::pair{ std::string_view{"receiverUser"}, &TypingRequest::receiverName }
    };

    // senderId: from JWT in Handler
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"chatroom_id"}, &TypingRequest::chatId }
    };

    static constexpr auto bools = std::array{
        std::pair{ std::string_view{"typing"}, &TypingRequest::isTyping }
    };
};


template<>
struct dtoFields<SeenDTO> {

    // userId: from JWT in Handler
    static constexpr auto ints = std::array{
        std::pair{ "chatroom_id", &SeenDTO::chatId },
        std::pair{ "last_seen_message_id", &SeenDTO::lastSeenMessageId }
    };
};
template<>
struct dtoFields<FetchDTO> {

    // userId: from JWT in Handler
    static constexpr auto ints = std::array{
        std::pair{ "chatroom_id", &FetchDTO::chatId },
        std::pair{ "chatIdentifier", &FetchDTO::chatIdentifier },
    };
};

template<>
struct dtoFields<SearchQuery> {
    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"searchedCharacters"}, &SearchQuery::searchedCharacters },
    };
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"searcherQueryId"}, &SearchQuery::searcherQueryId },
    };
};

template<>
struct dtoFields<MessageSearchQuery> {
    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"receiverUserName"}, &MessageSearchQuery::receiverUserName },
        std::pair{ std::string_view{"searchedText"}, &MessageSearchQuery::searchedText },
    };
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"searchQueryId"}, &MessageSearchQuery::searchQueryId },
    };
};

template<>
struct dtoFields<ReactionRequest> {
    static constexpr auto strings = std::array{
        std::pair{ std::string_view{"reaction"}, &ReactionRequest::reaction },
    };
    static constexpr auto ints = std::array{
        std::pair{ std::string_view{"messageId"}, &ReactionRequest::messageId },
        std::pair{ std::string_view{"chatroom_id"}, &ReactionRequest::chatRoomId },
    };
};

#endif