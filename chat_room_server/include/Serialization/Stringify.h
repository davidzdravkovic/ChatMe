#ifndef STRINGIFY_H
#define STRINGIFY_H
#include <string>
#include <vector>
#include "../NetworkAction/ApplicationResponse.h"
#include "../include/Serialization/ChatPreviewStringify.h"
#include "../include/Serialization/MessageStringify.h"
#include "../include/Serialization/FetchImagesIdStrinigify.h"





class Stringify {
public:
    std::vector<std::pair<std::string, std::string>>
    transform(const LoginSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const LoginFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const AuthSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const AuthFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const CreateSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const CreateFailurePayload&);

    std::vector<MessageString>
    transform(const FetchMessagesSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const FetchMessagesFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const SendMessageSuccessPayload&);

   std::vector<std::pair<std::string, std::string>>
   transform(const SendMessageAckPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const SendMessageFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const FirstMessageSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const FirstMessageFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const ChatRoomIdSuccessPayload &);

    std::vector<std::pair<std::string, std::string>>
    transform(const ChatRoomIdFailurePayload &);

    std::vector<std::pair<std::string, std::string>>
    transform(const PeerUserNotFoundPayload&);

    std::vector<ChatPreviewString>
    transform(const RecentChatRoomsSuccessPayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const RecentChatRoomsFailurePayload&);

    std::vector<std::pair<std::string, std::string>>
    transform(const ActiveStatusUpdatePayload&);

      std::vector<std::pair<std::string, std::string>>
      transform(const ActiveStatusUpdateFailurePayload&);

         std::vector<std::pair<std::string, std::string>>
      transform(const UploadProfilePicturePayload&);

         std::vector<std::pair<std::string, std::string>>
      transform(const UploadProfilePictureFailurePayload&);

         std::vector<std::pair<std::string, std::string>>
      transform(const TypingPayload&);

         std::vector<std::pair<std::string, std::string>>
      transform(const TypingFailurePayload&);
        
         std::vector<std::pair<std::string, std::string>>
      transform(const SeenSuccessPayload&);

         std::vector<std::pair<std::string, std::string>>
      transform(const UploadImageMessagePayload& payload);

       std::vector<FetchImagesIdString>
       transform ( const FetchImagesIdPayload& payload);

       std::vector<FetchImagesIdString>
       transform(const SearchQuerySuccessPayload& payload);

       std::vector<MessageString>
       transform(const MessageSearchSuccessPayload& payload);

       std::vector<std::pair<std::string, std::string>>
       transform(const ReactionSuccessPayload&);

       std::vector<std::pair<std::string, std::string>>
       transform(const ReactionFailurePayload&);

};
#endif
