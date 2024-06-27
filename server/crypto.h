#ifndef _CRYPTO_H_
#define _CRYPTO_H_
#include <openssl/md5.h>

#include "publiclib.h"

class Crypto {
 public:
  static Buffer MD5(const Buffer& text);
};
#endif