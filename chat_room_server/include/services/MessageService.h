#ifndef MESSAGESERVICE_H
#define MESSAGESERVICE_H
#include "../services/Manager.h"
#include "../SharedContext/OnlineUserRegistry.h"
#include "../include/NetworkAction/ApplicationResponse.h"
#include "../include/services/PresenceService.h"
#include "ResponseType.h"
#include <optional>

class MessageService {
public:
    MessageService(Manager& manager, OnlineUserRegistry& registry,PresenceService &service)
        : manager(manager), userRegistry(registry), presenceService(service) {}



     std::vector<ApplicationResponse> sendMessage(Message &mess, int issuerSessionId);
   
     std::vector<ApplicationResponse> sendFirstMessage(Message message, int issuerSessionId);

    ApplicationResponse makeSeen(int chatId,int userId, int lastSeenMessageId);
    // ApplicationResponse  getSeenState(int chatId,int userId);
   ApplicationResponse fetchMessages( std::string& senderUserName, std::string& receiverUserName,int limit,std::optional<int> beforeMessageId, std::optional<int> afterMessageId, int identifier, int issuerSessionId, std::optional<int> anchorMessageId = std::nullopt);

   /** In-chat message search; returns a MESSAGE_SEARCH_RESPONSE to the requesting session only. */
   ApplicationResponse searchMessages(std::string& senderUserName, std::string& receiverUserName, const std::string& text, int searchQueryId, int issuerSessionId);
   
    ApplicationResponse getRecentChats( int userId , int sessionId);
    ApplicationResponse fetchImagesId (int chatId, int userId, int chatIdentifier, int issuerSessionId);

    std::vector<ApplicationResponse> setReaction(
        int messageId,
        int chatRoomId,
        int userId,
        const std::string& userName,
        const std::string& reaction,
        int issuerSessionId);

private:
    Manager& manager;
    OnlineUserRegistry& userRegistry;
    PresenceService &presenceService;
};

#endif // MESSAGESERVICE_H
