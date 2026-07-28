#ifndef MESSAGESDB_H
#define MESSAGESDB_H
#include "./models/Messages.h"
#include "./Deserialization/DTO/ChatPreview.h"
#include <optional>
#include <pqxx/pqxx>





namespace MessagesDB {


std::vector<Message>  getOldMessages (const int &chatID,int limit,std::optional<int> beforeMessageId, pqxx::work &tx);
Message createMessage(const Message &message, pqxx::work &tx);
Message getMessage(const int &chatID, const int &userID, pqxx::work &tx);
std::vector<ChatPreview>  getLastMessagePerChat (const std::vector<int> & chatID ,int myUserID, pqxx::work &tx);
int getSenderIdForMessage( const int& chatID,const int& messageID, pqxx::work& tx);
std::vector<Message> buildAscending(const pqxx::result& r, bool reverse);
std::vector<Message> getAfter( int chatID,int afterMessageId,int limit,pqxx::work& tx);
std::vector<Message> getBefore(int chatID, int beforeMessageId, int limit,pqxx::work& tx); 
std::vector<Message> getLatest(int chatID, int limit, pqxx::work& tx);
std::optional<Message> getById(int chatID, int messageId, pqxx::work& tx);
/** Case-insensitive substring match on message content within a chat; newest first. */
std::vector<Message> searchInChat(int chatID, const std::string& text, int limit, pqxx::work& tx);
std::optional<int> getLastSentMessageId(int chatID, int userID, pqxx::work& tx);
Message insertingMessageImagePending( int chatroomId, int senderId,std::optional<int> mediaId, pqxx::work& tx);
std::optional<Message> updatingMessageImageReady(int mediaId, pqxx::work& tx);
std::vector<int> imagesId(int chatId,  pqxx::work& tx);

void bindReplyPreview(Message& message, pqxx::work& tx);
void bindReplyPreviews(std::vector<Message>& messages, int chatID, pqxx::work& tx);
void bindReplyPreviewsAll(std::vector<std::vector<Message>>& batches, int chatID, pqxx::work& tx);

void bindReactions(std::vector<Message>& messages, pqxx::work& tx);
void bindReactionsAll(std::vector<std::vector<Message>>& batches, pqxx::work& tx);

};

#endif