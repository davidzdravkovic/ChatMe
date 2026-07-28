#ifndef MANAGER_H
#define MANAGER_H
#include "./DataBase/DataBase.h"
#include "./repositories/UserDB.h"
#include "./repositories/ChatRoomDB.h"
#include "./repositories/ChatParticipantsDB.h"
#include "./repositories/ChatPrivateDB.h"
#include "./repositories/MessagesDB.h"
#include "./repositories/MessageReactionsDB.h"
#include "./repositories/MediaDB.h"
#include "./repositories/Transactions.h"
#include "./repositories/SeenDB.h"
#include "./models/ChatRoom.h"
#include "./Deserialization/DTO/UploadImageMessage.h"
#include "./models/Messages.h"
#include "./SharedResource/Job.h"
#include "./models/User.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <pqxx/pqxx> 
#include <thread>

class Manager {
    std::mutex universal;
    std::condition_variable cv;
    DataBasePool &connPool;
   // USED FOR THREADS WHEN CLIENT TARGET GETS THE MESSAGE BEFORE THE DB
    std::queue<Job> jobQueue;

    std::vector<std::vector<Message>> bindUserNames (std::vector<std::vector<Message>> &messages, Transaction &trans);
    public:

    Manager(DataBasePool &pool):  connPool(pool) {}
     std::optional<User> createUser(const User &user);
     int getUserId(const std::string& userName);

    Message createFirstMessage(Message &message);

    std::optional<std::vector<User>> searchChats(const int &userID);//For loading chat rooms option


   std::vector<std::vector<Message>> retreiveMessages( std::string& senderUserName, std::string& receiverUserName,int limit,std::optional<int> beforeMessageId, std::optional<int> afterMessageId, std::optional<int> anchorMessageId = std::nullopt);

   /** Case-insensitive in-chat message search; returns newest-first matches. */
   std::vector<Message> searchMessages(const std::string& senderUserName, const std::string& receiverUserName, const std::string& text, int limit = 50);

   Message sendingMessages( Message &message);

     std::optional<User> logIn(std::string userName, std::string password);

    Message getMessage(const int &chatID, const int &userID);//To retreive the other side message
    
    std::vector<User> getNames (const std::vector<int> &userID);
    void firstMessage(const Message &message);
    void asyncDBOperations();
    std::vector<ChatPreview>  getChatRooms(const int &userID);

        int createMedia(int userId,std::string mimeType, std::uint64_t fileSizeBytes);
        bool finalizeProfilePictureUpload(int userId,const int& uploadId);
        Message initialInsertionImageMessage(const UploadImageMessageRequest& upload);
        std::optional<SeenDB::SeenRow> CreateSeen(int chatId, int userId,int lastSeenMessageId);
        std::optional<SeenDB::SeenRow> getSeenState(int chatId, const std::string& userName);
        std::optional<SeenDB::SeenRow> getSeenStateByUserId(int chatId, int userId);
        int getOtherUserId(int chatId, int userId);
        Message finalizeImageMessageReady(const UploadImageMessageRequest& upload);
        std::vector<int> getImagesId(int chatId, int userId);

    /** Search users by username substring; excludes `excludeUserId`. Transaction scoped here. */
    std::vector<User> searchUsersByCharacters(const std::string& characters, int excludeUserId, int limit = 50);

    /** Upsert or remove (empty reaction) a user's reaction on a message in a chat. */
    bool setMessageReaction(int messageId, int chatRoomId, int userId, const std::string& reaction);

    /** Persists disconnect time; returns stored last_active_at. */
    std::optional<std::string> updateLastActiveAt(int userId);

};

#endif