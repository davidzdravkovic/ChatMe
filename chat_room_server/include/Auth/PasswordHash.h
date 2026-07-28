#pragma once

#include <string>

namespace PasswordHash {

/**
 * One-time library init (safe to call multiple times).
 * Throws std::runtime_error if libsodium cannot be initialized.
 */
void init();

/** Returns true if the stored value looks like a libsodium Argon2 hash string. */
bool looksHashed(const std::string& stored);

/** Hash plaintext password using libsodium's crypto_pwhash_str (Argon2id). */
std::string hash(const std::string& password);

/** Verify plaintext password against stored hash. Returns false on mismatch/invalid. */
bool verify(const std::string& password, const std::string& storedHash);

} // namespace PasswordHash

