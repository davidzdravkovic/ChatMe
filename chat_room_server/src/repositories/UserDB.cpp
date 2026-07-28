#include "../include/repositories/UserDB.h"
#include "../include/models/User.h"
#include <iostream>
#include <vector>
#include <string>

std::optional<User> UserDB::getUser(const std::string &username, const std::string &password, pqxx::work &tx) {
    try {
        pqxx::result r = tx.exec_params(
            "SELECT * FROM users WHERE username=$1 AND password=$2",
            username, password);

        if (r.empty())
            return std::nullopt;

        User u;
        u.userID = r[0]["user_id"].as<int>();
        u.userName = r[0]["username"].as<std::string>();
        u.password = r[0]["password"].as<std::string>();
        u.email = r[0]["email"].as<std::string>();
        u.name = r[0]["name"].as<std::string>();
        return u;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<User> UserDB::getUserByUsername(const std::string& username, pqxx::work& tx) {
    try {
        pqxx::result r = tx.exec_params("SELECT * FROM users WHERE username=$1", username);
        if (r.empty())
            return std::nullopt;

        User u;
        u.userID = r[0]["user_id"].as<int>();
        u.userName = r[0]["username"].as<std::string>();
        u.password = r[0]["password"].as<std::string>();
        u.email = r[0]["email"].as<std::string>();
        u.name = r[0]["name"].as<std::string>();
        return u;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

void UserDB::updatePassword(const std::string& username, const std::string& newPasswordValue, pqxx::work& tx) {
    try {
        tx.exec_params(
            "UPDATE users SET password=$2 WHERE username=$1",
            username, newPasswordValue
        );
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

void UserDB :: createUser(const User &user, pqxx::work &tx) {
 try {
       
 
   
     tx.exec_params(
            "INSERT INTO users (username, password, email, name) VALUES ($1,$2,$3,$4)",
            user.userName,user.password,user.email,user.name
        );
       
         
    }
    
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;

    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;
 
    }
   

}

bool UserDB :: uniqueness (const std::string &username, pqxx::work &tx) {
try {    

        pqxx::result r=tx.exec_params("SELECT * FROM users WHERE username = $1",username);

        if(!r.empty()) {
     return false; 
        }
}
  catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;

    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;

    }
   
return true;

}

std::optional<std::vector<User>> UserDB :: getUserNameByID (const std::vector<int> &usersID, pqxx::work &tx) {


     std::vector<User> users;
   
  try {
 
    for(int i=0; i<usersID.size(); i++) {
      
    int userID = usersID[i];  
    pqxx::result r = tx.exec_params("SELECT username, user_id FROM users WHERE user_id=$1",userID); 
     if(!r.empty())
      {
      User user;
      user.userName = r[0] ["username"].as<std::string>();
      user.userID = r[0] ["user_id"].as<int>();
      users.push_back(user);
     }

  }
  return users;

}
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;
 return std::nullopt;

    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;
 return std::nullopt; 

    }
  
}

std::optional<std::vector<int>> UserDB :: getIDByUserName (const std::vector<std::string> &usersNames, pqxx::work &tx) {

     std::vector<int> usersID;

  try {


    for(int i=0; i<usersNames.size(); i++) {
    std::string userName = usersNames[i];  
    pqxx::result r = tx.exec_params("SELECT user_id FROM users WHERE username=$1",userName); 
     if(!r.empty())
      {
      int userID;
      userID = r[0] ["user_id"].as<int>();
      usersID.push_back(userID);
     }

  }
  return usersID;

}
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;

 return std::nullopt; 

    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;

 return std::nullopt; 

    }
  
}

int UserDB :: getID (const std::string &userName, pqxx::work &tx) {

      int userID=-1;
    
  try {

    
    pqxx::result r = tx.exec_params("SELECT user_id FROM users WHERE username=$1",userName); 
     if(!r.empty())
      {
     
      userID = r[0] ["user_id"].as<int>();
 
     }

    }
    catch(pqxx::sql_error &e) {
 std::cerr<<"SQL error: "<<e.what()<<std::endl;
 std::cerr<<"Querie error: "<<e.query()<<std::endl;



    }
    catch(std::exception &e) {
  std::cerr<<"Error: "<<e.what()<<std::endl;

 

    }
    
  return userID;
  
}


std::vector<std::string> UserDB :: getUserById (const std::vector<int> &usersID, pqxx::work &tx) {


     std::vector<std::string> userNames;
   
  try {
 
    for(int i=0; i<usersID.size(); i++) {
      
    int userID = usersID[i];  
    pqxx::result r = tx.exec_params("SELECT username, user_id FROM users WHERE user_id=$1",userID); 
     if(!r.empty())
      {
    
      std::string userName = r[0] ["username"].as<std::string>();
      userNames.push_back(userName);
     }

  }
  return userNames;

    } catch (pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Querie error: " << e.query() << std::endl;
        return {};
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};
    }
}
void UserDB::updateProfilePicture( int userId,const std::string& uploadId,  pqxx::work& tx) {
    try {
        tx.exec_params(
            "UPDATE users "
            "SET profile_picture_id = $1, "
            "    profile_picture_updated_at = now() "
            "WHERE user_id = $2",
            uploadId, userId
        );
    }
    catch (pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        throw;
    }
}

namespace {

std::string trimSearchInput(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return {};
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

/** Escape `\`, `%`, `_` so the prefix is matched literally before the trailing `%` in ILIKE. */
std::string escapeForIlike(const std::string& raw) {
    std::string out;
    out.reserve(raw.size() + 8);
    for (char c : raw) {
        if (c == '\\' || c == '%' || c == '_')
            out += '\\';
        out += c;
    }
    return out;
}

} // namespace

std::vector<User> UserDB::searchUsersByCharacters(const std::string& characters,
                                                  int excludeUserId,
                                                  int limit,
                                                  pqxx::work& tx) {
    std::vector<User> out;
    const std::string trimmed = trimSearchInput(characters);
    if (trimmed.empty() || limit <= 0)
        return out;

    // Prefix match: pattern is `<trimmed>%` only; `trimmed` is from the DTO `searchedCharacters` (not a literal).
    const std::string pattern = escapeForIlike(trimmed) + "%";

    try {
        pqxx::result r = tx.exec_params(
            "SELECT user_id, username FROM users "
            "WHERE username ILIKE $1 ESCAPE '\\' AND user_id <> $2 "
            "ORDER BY username ASC "
            "LIMIT $3",
            pattern,
            excludeUserId,
            limit);

        for (const auto& row : r) {
            User u;
            u.userID = row["user_id"].as<int>();
            u.userName = row["username"].as<std::string>();
            out.push_back(std::move(u));
        }
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query error: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return out;
}

std::optional<std::string> UserDB::updateLastActiveAt(int userId, pqxx::work& tx) {
    try {
        pqxx::result r = tx.exec_params(
            "UPDATE users SET last_active_at = now() WHERE user_id = $1 RETURNING last_active_at",
            userId);

        if (r.empty() || r[0]["last_active_at"].is_null())
            return std::nullopt;

        return r[0]["last_active_at"].as<std::string>();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query error: " << e.query() << std::endl;
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}
