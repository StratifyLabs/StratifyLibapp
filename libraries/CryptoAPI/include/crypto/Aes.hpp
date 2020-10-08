/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
#ifndef CRYPTOAPI_CRYPTO_AES_HPP_
#define CRYPTOAPI_CRYPTO_AES_HPP_

#include <sos/api/crypt_api.h>

#include "api/api.hpp"

#include "fs/File.hpp"

#if defined __link && !defined CRYPT_AES_API_REQUEST
#include <mbedtls_api.h>
#define CRYPT_AES_API_REQUEST &mbedtls_crypt_aes_api
#endif

namespace crypto {

class Aes : public api::Object {
public:
  using InitializationVector = var::Array<u8, 16>;
  using Iv = InitializationVector;

  Aes();
  ~Aes();

  bool is_valid() const { return m_context != nullptr; }

  Aes &set_key(const var::View &key);

  Aes &set_initialization_vector(const var::View &value);

  const InitializationVector &initialization_vector() const {
    return m_initialization_vector;
  }

  class Crypt {
    API_AF(Crypt, fs::File *, plain, nullptr);
    API_AF(Crypt, fs::File *, cipher, nullptr);
  };

  Aes &encrypt_ecb(const Crypt &options);
  Aes &decrypt_ecb(const Crypt &options);

  Aes &encrypt_cbc(const Crypt &options);
  Aes &decrypt_cbc(const Crypt &options);

#if 0 // not yet implemented
  Aes &encrypt_ctr(const CryptOptions &options);
  Aes &decrypt_ctr(const CryptOptions &options);
#endif

private:
  using Api = api::Api<crypt_aes_api_t, CRYPT_AES_API_REQUEST>;
  static Api m_api;

  void *m_context = nullptr;
  InitializationVector m_initialization_vector;

  static Api &api() { return m_api; }
  int initialize();
  int finalize();
};

} // namespace crypto

#endif // CRYPTOAPI_CRYPTO_AES_HPP_