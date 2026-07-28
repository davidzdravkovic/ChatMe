#ifndef CHATRETIEVE_H
#define CHATRETIEVE_H
#include <string>
#include <optional>


struct ChatRetieve {
  int sessionId;
  int senderId;
  int limit;
  int identifier;
  std::optional<int> beforeMessageId;
  std::optional<int> afterMessageId;
  /** When set (search jump), the initial window is centered on this message id instead of lastSeen/lastSent. */
  std::optional<int> anchorMessageId;
  std::string receiverUserName;
  std::string senderUserName;


};

   #endif