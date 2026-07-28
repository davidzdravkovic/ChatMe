#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "../include/models/Messages.h"
#include "./MessageStringify.h"
#include "./Deserialization/DTO/ChatPreview.h"
#include "./Serialization/FetchImagesIdStrinigify.h"
#include "./ChatPreviewStringify.h"
#include "ResponseType.h"
#include <vector>
#include <string>
#include <utility>
#include <cstdint>

class Serialization {
std::string response(const ResponseType &resType);
int sessionID;
public:   
void setID(int id) {sessionID = id;}
bool isNumber(const std::string &second);
 std::vector<uint8_t> serialize(std::vector<std::pair<std::string, std::string>> fields, const int &status,const ResponseType &resType);
 std::vector<uint8_t> serialize(std::vector<MessageString> mString, const int &status,const ResponseType &resType);
 std::vector<uint8_t> serialize(std::vector<ChatPreviewString> mString, const int &status,const ResponseType &resType);
 std::vector<uint8_t> serialize(std::vector<FetchImagesIdString> mString, const int &status,const ResponseType &resType);

 int getId() {return sessionID;}

};

#endif