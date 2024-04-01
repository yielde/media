#include "ssltool.h"
#include <QDebug>
// openssl genrsa -out rsa_private_key.pem 1024
const char* SERVER_RSA_PRIVATE = "-----BEGIN PRIVATE KEY-----\n"
                                 "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCjPPuKTX9VLF84\n"
                                 "AtMXn31+BGdp+auNmLHIBQICOHTyeOa4XqV4D3VYrpEEQJzmtmqNCwAgWSINDfl6\n"
                                 "8ER/UTQgS+8RdGnTGZqJtsjr8z/ETWPcXzx2WbVQpkvAjKOHgwESjM+4NDyuzUa0\n"
                                 "wENZ6KLgG1GqbM+8mIs5RGsmd8Tio5vEtgUCzrlDXA35vw/jKnxC81KYghOr+Ds3\n"
                                 "CxAJSTVv4lc83BCE7MPfClwoJw+L4Fd1ARFMb3y5GBvuI2TSU1XYElezhkSCQ9Eo\n"
                                 "WoEavPZhFDj3VN0HxH1cLdrDyKWfzKWDuroj3yq4jJJcU9SwOVvIiGimXcZUleBz\n"
                                 "PvoK+gBfAgMBAAECggEADN26H8kC78YrDSGf3L1L6VpUglRU8rilzpOieGVVxmTE\n"
                                 "HqULGr7xK1e7A2BKXZ7kIBDNZj9QQUIoN0nnxDc0yhTiF3ia/LIIbbFT/dDy5jiJ\n"
                                 "Ve3FuLhSY7RvxRHws7YeKAWkO6sWW1q/p3B9/oCpEcvlcG7aIs3NL2rWnTYUdR56\n"
                                 "4QZiFRMOTAKB9w5fixU4oEkm3H/r17JqXy8XMJOrNdw6vCPwDgHqaosRVc1KSP/K\n"
                                 "mGtDJHkwMLr17rXPFLhgxrwHpXv5/84e+SQZ4nEL+Fgpw5uPuq8kTGdiIYqlqUo7\n"
                                 "Bz7JJi2Zjip8ImyY/MPURFp/6hVbpz9+IGo8VO0GrQKBgQDUqaQRQLagLzH4J7Xw\n"
                                 "A0lfThAWNPsAF4nL/LkKd+Q9R9QbzX9pTofzOotc8AAE6gD9y0+thejS4dCoUR/2\n"
                                 "z912g9NxQFcOABz3bDd7z3j9+U2EsEVzCBCh7oii4jo8kBxNpj+C9FK78xhhWGjK\n"
                                 "Lb+dPo0w8MwlGMkB4oD0YUTx8wKBgQDEgO1F1LjAcC5ynhrvrhVpswJaHfTu4AWQ\n"
                                 "IWTPo4pMW1P1MNV+meLZSvFWQJ74rWqCRDoMF8mPjbmnUf42J47ND5DT/QDmewJc\n"
                                 "YsCeblUSoLaqk0RHBbr+nQPsxvqDujqMkq4rBUlkTRJYkX+TU2DWyhLLcTZ0B65f\n"
                                 "Jv0sb26m5QKBgEvhG8FsLb2KGXJJ+Qjio1N2Lxc9YDHwNCKs0fmCPdX/wxcIg+W3\n"
                                 "N5r50LAgkmEufZwQ5yUrGp/kIudrytPt5z9aWh+WrXk5YRyHaGSs6qb0RUyK+LzK\n"
                                 "q+Sbj/VOcykY4oBySUcYlkypuJFDUmCRYkVhBE/qG21BIrehchHAgzExAoGBAKnc\n"
                                 "NL6XWjjnAA+OAgTLe7EJz2+s04wl1Ek5xJYEbUkB7TpWw0YTqddhg+qmI4UdCqOe\n"
                                 "bHsFmPNhdGlhZBjV6wfxe6Tz3/JbxLetmYmaICnhCOW8NVobZwAvJDRp8CUsNu5K\n"
                                 "4QlJIZ71THWJGDl26o/gz4xSydpUxXN1FYZysLjNAoGAdBohRoqYEvuWkz0Q/tRK\n"
                                 "mxYiXYAelNKbygNsU36ZUdoGUaIkxPSYkDpMRcT3kAT7j9zLKZcbusOF/qRFIqVF\n"
                                 "QLuzksiCgpYIdt9P4txBY7wZ4TjGUmoISQqA8CKqMMksn0di6YpCgxGtiOQzprl7\n"
                                 "/U1UxE2N018Ng2pATJQPfi4=\n"
                                 "-----END PRIVATE KEY-----\n";
// openssl rsa -in rsa_private_key.pem  -pubout -out rsa_public_key.pem
const char* SERVER_RSA_PUBLIC
    = "-----BEGIN PUBLIC KEY-----\n"
      "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAozz7ik1/VSxfOALTF599\n"
      "fgRnafmrjZixyAUCAjh08njmuF6leA91WK6RBECc5rZqjQsAIFkiDQ35evBEf1E0\n"
      "IEvvEXRp0xmaibbI6/M/xE1j3F88dlm1UKZLwIyjh4MBEozPuDQ8rs1GtMBDWeii\n"
      "4BtRqmzPvJiLOURrJnfE4qObxLYFAs65Q1wN+b8P4yp8QvNSmIITq/g7NwsQCUk1\n"
      "b+JXPNwQhOzD3wpcKCcPi+BXdQERTG98uRgb7iNk0lNV2BJXs4ZEgkPRKFqBGrz2\n"
      "YRQ491TdB8R9XC3aw8iln8ylg7q6I98quIySXFPUsDlbyIhopl3GVJXgcz76CvoA\n"
      "XwIDAQAB\n"
      "-----END PUBLIC KEY-----\n";

SslTool::SslTool()
{
    BIO* bio = BIO_new_mem_buf(SERVER_RSA_PUBLIC, -1); // 创建只读BIO
    if (bio == NULL) {
        throw "load public key failed";
    }
    rsa_public = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    bio = BIO_new_mem_buf(SERVER_RSA_PRIVATE, -1);
    if (bio == NULL) {
        throw "laod private key failed";
    }
    rsa_private = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
}

QByteArray SslTool::rsaEncode(const QByteArray& buffer)
{
    QByteArray result;
    result.resize(buffer.size() * 3); // RSA_PKCS1_PADDING填充，加密明文的长度不能超过秘钥长度减11个字节
    memset(result.data(), 0, result.size());
    int ret = RSA_public_encrypt((int)buffer.size(),
        (const unsigned char*)buffer.data(),
        (unsigned char*)result.data(),
        rsa_public, RSA_PKCS1_PADDING);
    if (ret == -1) {
        result.clear();

        return result; // 失败返回""
    }
    result.resize(ret);
    return result;
}

QByteArray SslTool::rsaDecode(const QByteArray& input)
{
    QByteArray result;
    result.resize(input.size());
    int ret = RSA_private_decrypt(input.size(),
        (const unsigned char*)input.data(),
        (unsigned char*)result.data(),
        rsa_private, RSA_PKCS1_PADDING);
    if (ret == -1) {
        result.clear();
        return result;
    }
    return result;
}
