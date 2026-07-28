#include "../include/services/Manager.h"
#define LOG_STEP(msg) do {} while(0)
#define LOG_ERR(msg) do {} while(0)

#include "../include/models/User.h"
#include "../include/models/Messages.h"
#include <vector>
#include <pqxx/pqxx>
#include <iostream>
#include "../include/Auth/PasswordHash.h"



std::vector<ChatPreview> Manager::getChatRooms(const int& userID) {
    Transaction trans(connPool);
    std::vector<int> chatRoomsID = ChatParticipantsDB::getChatRoomsID(userID, trans.get());
    std::vector<ChatPreview> lastMessages =
        MessagesDB::getLastMessagePerChat(chatRoomsID, userID, trans.get());
    trans.commit();
    return lastMessages;
}

std::vector<User> Manager::searchUsersByCharacters(const std::string& characters, int excludeUserId, int limit) {
    Transaction trans(connPool);
    std::vector<User> users = UserDB::searchUsersByCharacters(characters, excludeUserId, limit, trans.get());
    trans.commit();
    return users;
}

std::optional<User> Manager::createUser(const User &user) {
    Transaction trans(connPool);
    if (!UserDB::uniqueness(user.userName, trans.get()))
        return std::nullopt;

    User toCreate = user;
    // Store password hash in the same DB column to avoid schema changes.
    toCreate.password = PasswordHash::hash(user.password);

    UserDB::createUser(toCreate, trans.get());
    auto userCreated = UserDB::getUserByUsername(user.userName, trans.get());
    if (!userCreated.has_value()) {
        trans.get().abort();
        return std::nullopt;
    }
    trans.commit();
    return userCreated;
}


std::optional<std::vector<User>> Manager :: searchChats(const int &userID) {

    Transaction trans(connPool);

    std::optional<std::vector<int>> ID = ChatParticipantsDB::getUsersID(userID, trans.get());
    if (!ID)
        return std::nullopt;

    std::vector<int> usersID = ID.value();
    std::optional<std::vector<User>> userNames = UserDB::getUserNameByID(usersID, trans.get());
    if (!userNames)
        return std::nullopt;

    trans.commit();
    return userNames;
}

std::vector<std::vector<Message>> Manager::retreiveMessages(std::string& senderUserName,
                                                            std::string& receiverUserName,
                                                            int limit,
                                                            std::optional<int> beforeMessageId,
                                                            std::optional<int> afterMessageId,
                                                            std::optional<int> anchorMessageId) {
    Transaction trans(connPool);

    int senderUserId = UserDB::getID(senderUserName, trans.get());
    int receiverUserId = UserDB::getID(receiverUserName, trans.get());

    int chatID = ChatParticipantsDB::getChatID(senderUserId, receiverUserId, trans.get());

    auto lastSeen = SeenDB::fetchSeen(chatID, senderUserId, trans.get());
    auto lastSent = MessagesDB::getLastSentMessageId(chatID, senderUserId, trans.get());

    std::optional<int> anchor;
    if (anchorMessageId)
        // Search jump: center the initial window on the clicked message.
        anchor = anchorMessageId;
    else if (lastSeen && lastSent)
        anchor = std::max(lastSeen->lastSeenMessageId, *lastSent);
    else
        anchor = lastSeen ? std::optional<int>{lastSeen->lastSeenMessageId} : lastSent;

    std::vector<std::vector<Message>> allMessages;
    allMessages.resize(2);
    std::vector<Message> messages;

    // INITIAL FETCH WITH 0,0 ON BEFORE AND AFTER
    if (!beforeMessageId && !afterMessageId) {

        if (anchor) {
            std::vector<Message> lastMessages;

            // Before anchor
            auto older = MessagesDB::getBefore(chatID, *anchor, limit, trans.get());
            // The anchor message itself
            auto anchorValue = MessagesDB::getById(chatID, *anchor, trans.get());
            // After anchor
            auto newer = MessagesDB::getAfter(chatID, *anchor, limit, trans.get());

            messages.reserve(older.size() + (anchorValue ? 1 : 0) + newer.size());

            messages.insert(messages.end(), older.begin(), older.end());

            if (anchorValue)
                messages.push_back(*anchorValue);

            messages.insert(messages.end(), newer.begin(), newer.end());
            lastMessages = MessagesDB::getLatest(chatID, limit, trans.get());

            // Inserting initial messages at index 0 and last at index 1
            allMessages[0] = messages;
            allMessages[1] = lastMessages;

        } else {
            messages = MessagesDB::getLatest(chatID, limit, trans.get());
            // |initial messages| |barrier| |last messages|
            allMessages[0] = messages;
            allMessages[1] = messages;
        }
    }

    else if (beforeMessageId) {
        messages = MessagesDB::getBefore(chatID, *beforeMessageId, limit, trans.get());
        allMessages[0] = messages;
    }

    else {
        messages = MessagesDB::getAfter(chatID, *afterMessageId, limit, trans.get());
        allMessages[0] = messages;
    }
    // Modifies allMessagas - attaches userName to each message
    allMessages = bindUserNames(allMessages, trans);
    MessagesDB::bindReplyPreviewsAll(allMessages, chatID, trans.get());
    MessagesDB::bindReactionsAll(allMessages, trans.get());

    trans.commit();
    return allMessages;
}

std::vector<Message> Manager::searchMessages(const std::string& senderUserName,
                                             const std::string& receiverUserName,
                                             const std::string& text,
                                             int limit) {
    Transaction trans(connPool);

    int senderUserId = UserDB::getID(senderUserName, trans.get());
    int receiverUserId = UserDB::getID(receiverUserName, trans.get());

    if (senderUserId < 0 || receiverUserId < 0) {
        trans.commit();
        return {};
    }

    int chatID = ChatParticipantsDB::getChatID(senderUserId, receiverUserId, trans.get());
    std::vector<Message> hits = MessagesDB::searchInChat(chatID, text, limit, trans.get());

    trans.commit();
    return hits;
}

Message Manager::createFirstMessage(Message& message) {
    LOG_STEP("createFirstMessage() START");

    Transaction trans(connPool);
    LOG_STEP("Transaction created");

    int senderId = message.senderID;
    LOG_STEP("SenderId = " << senderId);

    int receiverId = UserDB::getID(message.receiverUserName, trans.get());
    LOG_STEP("ReceiverId resolved = " << receiverId);

    int privateChatId = -1;
    int chatRoomId = -1;

    try {
        pqxx::subtransaction sp(trans.get(), "create_private_chat");

        privateChatId = ChatPrivateDB::createPrivateChat(senderId, receiverId, trans.get());

        chatRoomId = ChatRoomDB::createChatRoom(2, trans.get());

        ChatPrivateDB::bindChatroom(privateChatId, chatRoomId, trans.get());

        ChatParticipantsDB::insertChatParticipants(senderId, receiverId, chatRoomId, trans.get());

        sp.commit();
    } catch (const pqxx::unique_violation&) {
        privateChatId = ChatPrivateDB::findPrivateChat(senderId, receiverId, trans.get()).value();

        chatRoomId = ChatPrivateDB::getChatroomId(privateChatId, trans.get());
    }

    LOG_STEP("Creating first message");

    message.chatRoomID = chatRoomId;
    Message created = MessagesDB::createMessage(message, trans.get());
    MessagesDB::bindReplyPreview(created, trans.get());
    created.receiverId = receiverId;

    LOG_STEP("Message created, id = " << created.time);

    LOG_STEP("Committing transaction");
    trans.commit();

    LOG_STEP("Transaction committed successfully");
    LOG_STEP("createFirstMessage() END");

    return created;
}

Message Manager::sendingMessages(Message& message) {
    Transaction trans(connPool);

    Message theMessage;
    theMessage = MessagesDB::createMessage(message, trans.get());
    MessagesDB::bindReplyPreview(theMessage, trans.get());
    trans.commit();
    return theMessage;
}
void Manager :: firstMessage (const Message &message) {

  std::lock_guard<std::mutex> lock(universal);

    Job job;
    job.type = JobType::INSERT_FIRST_MESSAGE;
    job.message = message;

    jobQueue.push(job);
    cv.notify_one();

 
}



std::vector<User> Manager :: getNames (const std::vector<int> &usersID) {

    Transaction trans(connPool);
   
   std::optional<std::vector<User>> user  = UserDB::getUserNameByID(usersID,trans.get());
   std::vector<User> &users = user.value();

   trans.commit();
   return users;
   
}

std::optional<User> Manager::logIn(std::string userName, std::string password) {
    Transaction trans(connPool);
    auto user = UserDB::getUserByUsername(userName, trans.get());
    if (!user.has_value())
        return std::nullopt;

    const std::string stored = user->password;
    bool ok = false;
    if (PasswordHash::looksHashed(stored)) {
        ok = PasswordHash::verify(password, stored);
    } else {
        // Legacy plaintext row: verify once and upgrade to hash.
        ok = (password == stored);
        if (ok) {
            try {
                const std::string newHash = PasswordHash::hash(password);
                UserDB::updatePassword(userName, newHash, trans.get());
                user->password = newHash;
            } catch (...) {
                // If upgrade fails, still allow login (legacy) but keep DB unchanged.
            }
        }
    }

    if (!ok)
        return std::nullopt;
    trans.commit();
    return user;
}

int Manager::createMedia(int userId,std::string mimeType, std::uint64_t fileSizeBytes) {
    //NAMES ARE FOR PROFILE PICTURE UPLOAD BUT FUNCTION IS USED FOR IMAGE MESSAGE UPLOAD TOO
   Transaction trans(connPool);
     int mediaId = MediaDB::createTempMedia( userId, mimeType,fileSizeBytes, trans.get());
     trans.commit();
    return mediaId;
}

bool Manager::finalizeProfilePictureUpload(int userId,const int& uploadId){
    Transaction trans(connPool); 

   
    if (!MediaDB::markMediaReady(userId, uploadId, trans.get())) {
        trans.get().abort();
        return false;
    }


    if (!MediaDB::attachProfilePicture(userId, uploadId, trans.get())) {
        trans.get().abort();
        return false;
    }

    trans.commit();
    return true;
}
Message Manager::initialInsertionImageMessage(const UploadImageMessageRequest& upload) {
    Transaction trans(connPool);


    if (!MediaDB::markMediaReady( upload.userId,  upload.uploadId,  trans.get()  )) {
        trans.get().abort();
    }

    int receiverUserId = UserDB::getID(upload.receiverUserName,trans.get());
    int chatroomId = ChatParticipantsDB::getChatID(upload.userId,receiverUserId,trans.get());
 
    Message message =  MessagesDB::insertingMessageImagePending(chatroomId,upload.userId, upload.uploadId, trans.get());


    trans.commit();
    return message;
}

Message Manager::finalizeImageMessageReady(const UploadImageMessageRequest& upload)
{
    Transaction trans(connPool);


    // 2️⃣ update message status PENDING_MEDIA → READY
    auto opt = MessagesDB::updatingMessageImageReady(upload.uploadId, trans.get());

    if (!opt.has_value()) {
        trans.get().abort();
        throw std::runtime_error("Pending image message not found");
    }

    Message ready = *opt;
    MessagesDB::bindReplyPreview(ready, trans.get());
    trans.commit();
    return ready;
}




int Manager::getOtherUserId(int chatId, int userId) {

    Transaction trans(connPool);

    int otherUserId =ChatParticipantsDB::getOtherParticipant( chatId, userId, trans.get());

    trans.commit();
    return otherUserId;   
}

std::optional<SeenDB::SeenRow> Manager::CreateSeen(int chatId, int userId, int lastSeenMessageId)
{
    Transaction trans(connPool);

    int senderId = MessagesDB::getSenderIdForMessage(chatId, lastSeenMessageId, trans.get());

    if (senderId == -1) {
        trans.commit();
        return std::nullopt;
    }

    if (senderId == userId) {
        trans.commit();
        return std::nullopt;
    }

    auto row = SeenDB::upsertLastSeenMessage(chatId, userId, lastSeenMessageId, trans.get());

    trans.commit();
    return row;
}

std::optional<SeenDB::SeenRow> Manager::getSeenState(int chatId, const std::string& userName)
{
    Transaction trans(connPool);

    int userId = UserDB::getID(userName, trans.get());
    auto row = SeenDB::fetchSeen(chatId, userId, trans.get());

    trans.commit();
    return row;
}

std::optional<SeenDB::SeenRow> Manager::getSeenStateByUserId(int chatId, int userId)
{
    Transaction trans(connPool);
    auto row = SeenDB::fetchSeen(chatId, userId, trans.get());
    trans.commit();
    return row;
}

std::vector<int> Manager::getImagesId(int chatId, int userId) {

    Transaction trans(connPool);

    std::vector<int> imagesId = MessagesDB::imagesId(chatId, trans.get());
    trans.commit();
    return imagesId;
}

int Manager::getUserId(const std::string& userName) {
    
    Transaction trans(connPool);
    int userId = UserDB::getID(userName, trans.get());
    trans.commit();
    return userId;
}
//Refactor this code to more intuitive binding by calling an API returning for each element a userName for provided userId 
std::vector<std::vector<Message>> Manager :: bindUserNames (std::vector<std::vector<Message>> &messages,Transaction &trans) {  //From userId extract usernames
    std::vector<int> userIdsFirstMessage;
    userIdsFirstMessage.reserve(messages[0].size());
  


    for (const auto& m : messages[0])
        userIdsFirstMessage.push_back(m.senderID);

    auto userNamesFirst = UserDB::getUserById(userIdsFirstMessage, trans.get());

    for (size_t i = 0; i < messages[0].size(); ++i)
        messages[0][i].senderUserName = userNamesFirst[i];

     if(messages[1].empty()) return messages; 

       std::vector<int> userIdsSecondMessage;
       userIdsSecondMessage.reserve(messages[1].size());

        for (const auto& m : messages[1])
        userIdsSecondMessage.push_back(m.senderID);

    auto userNamesSecond = UserDB::getUserById(userIdsSecondMessage, trans.get());

    for (size_t i = 0; i < messages[1].size(); ++i)
        messages[1][i].senderUserName = userNamesSecond[i];
   

        return messages;

   
}

bool Manager::setMessageReaction(
    int messageId,
    int chatRoomId,
    int userId,
    const std::string& reaction)
{
    if (messageId <= 0 || chatRoomId <= 0 || userId <= 0)
        return false;

    Transaction trans(connPool);

    if (!MessagesDB::getById(chatRoomId, messageId, trans.get()))
        return false;

    if (getOtherUserId(chatRoomId, userId) < 0)
        return false;

    bool ok = true;
    if (reaction.empty())
        MessageReactionsDB::remove(messageId, userId, trans.get());
    else
        ok = MessageReactionsDB::upsert(messageId, userId, reaction, trans.get());

    if (!ok)
        return false;

    trans.commit();
    return true;
}

std::optional<std::string> Manager::updateLastActiveAt(int userId) {
    Transaction trans(connPool);
    auto lastActiveAt = UserDB::updateLastActiveAt(userId, trans.get());
    trans.commit();
    return lastActiveAt;
}