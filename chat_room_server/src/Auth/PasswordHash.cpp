#include "../../include/Auth/PasswordHash.h"

#include <stdexcept>

#ifdef CHAT_ENABLE_PASSWORD_HASH
#include <sodium.h>
#endif

namespace PasswordHash {

void init() {
#ifdef CHAT_ENABLE_PASSWORD_HASH
  if (sodium_init() < 0) {
    throw std::runtime_error("libsodium initialization failed");
  }
#else
  // Password hashing disabled at build-time.
#endif
}

bool looksHashed(const std::string& stored) {
  // libsodium crypto_pwhash_str uses a "$argon2id$..." format.
  return stored.rfind("$argon2id$", 0) == 0 || stored.rfind("$argon2i$", 0) == 0;
}

std::string hash(const std::string& password) {
#ifdef CHAT_ENABLE_PASSWORD_HASH
  init();
  if (password.empty())
    throw std::runtime_error("Cannot hash empty password");

  char out[crypto_pwhash_STRBYTES];
  // MODERATE is a good default for servers; adjust later if needed.
  if (crypto_pwhash_str(out,
                        password.c_str(),
                        static_cast<unsigned long long>(password.size()),
                        crypto_pwhash_OPSLIMIT_MODERATE,
                        crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
    throw std::runtime_error("Password hashing failed (out of memory?)");
  }
  return std::string(out);
#else
  // If hashing is disabled, return plaintext (legacy). This should not be used in production.
  return password;
#endif
}

bool verify(const std::string& password, const std::string& storedHash) {
#ifdef CHAT_ENABLE_PASSWORD_HASH
  init();
  if (password.empty() || storedHash.empty())
    return false;
  return crypto_pwhash_str_verify(storedHash.c_str(),
                                  password.c_str(),
                                  static_cast<unsigned long long>(password.size())) == 0;
#else
  // Legacy plaintext compare when hashing is disabled.
  return password == storedHash;
#endif
}

} // namespace PasswordHash

