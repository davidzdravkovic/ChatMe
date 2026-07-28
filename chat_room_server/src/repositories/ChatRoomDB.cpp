#include "../include/repositories/ChatRoomDB.h"
#include <vector>
#include <iostream>


int ChatRoomDB :: createChatRoom (const int &sizeRoom, pqxx::work &tx) {

    std::string type;
     if(sizeRoom==2) {
      type="DirectMessage";
     }
     else if (sizeRoom>2) {
      type="Group";
     }
      
    try {
        pqxx::result r = tx.exec(
            "INSERT INTO chatrooms (type) VALUES (" + tx.quote(type) + ") RETURNING id;");

        int chatroomID = r[0][0].as<int>();
        return chatroomID;
    } catch (pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return -1;
}