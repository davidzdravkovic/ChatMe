#ifndef USERDB_H
#define USERDB_H
#include <vector>
#include <optional>
#include "./models/User.h"
#include <pqxx/pqxx>

namespace UserDB {
 
// Legacy API (plaintext compare) - kept for compatibility but should not be used by new code.
std::optional<User> getUser(const std::string &username, const std::string &password, pqxx::work &tx);

// Fetch by username (used for hashed password verification).
std::optional<User> getUserByUsername(const std::string& username, pqxx::work& tx);

// Update stored password field (e.g. upgrade plaintext -> hash).
void updatePassword(const std::string& username, const std::string& newPasswordValue, pqxx::work& tx);

bool uniqueness(const std::string &username, pqxx::work &tx);

int getID(const std::string &userName, pqxx::work &tx);

std::optional<std::vector<User>> getUserNameByID(const std::vector<int> &usersID, pqxx::work &tx);

std::vector<std::string> getUserById(const std::vector<int> &usersID, pqxx::work &tx);

std::optional<std::vector<int>> getIDByUserName(const  std::vector<std::string> &usersNames, pqxx::work &tx);

void createUser(const User &user, pqxx::work &tx);

void updateProfilePicture(int userId,const std::string& uploadId,pqxx::work& tx);

/**
 * Username prefix search (case-insensitive): `username ILIKE '<prefix>%'` where `<prefix>` comes from the request (`characters`).
 * Excludes `excludeUserId`. `characters` trimmed; empty → empty result. `limit` caps max rows returned.
 */
std::vector<User> searchUsersByCharacters(const std::string& characters,
                                          int excludeUserId,
                                          int limit,
                                          pqxx::work& tx);

/** Sets last_active_at to now(); returns the stored timestamptz string. */
std::optional<std::string> updateLastActiveAt(int userId, pqxx::work& tx);

};

#endif

///
///  Manager manager(UserRepo,MessageRepo)
///
///
///