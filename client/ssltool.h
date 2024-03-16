#ifndef SSLTOOL_H
#define SSLTOOL_H

extern "C" {
#include "openssl/types.h"
}

#include <QByteArray>
#include <string>

using std::string;

class BigNum {
public:
    BigNum();
    BigNum(const BigNum& bm);
    BigNum(const string& bm, bool is_base64 = true);
    ~BigNum();
    operator BIGNUM*();
    operator string();

    BigNum& operator=(const string& bm);
    BigNum& operator=(const BigNum& bm);

    BigNum& fromBase64(const string& bm);
    BigNum& fromHex(const string& bm);
    BigNum& fromBinary(const string& bm);
    string toBinary();
    static string base64Decode(const string& input, bool newline = false);
    static string base64Encode(const string& buffer, bool newline = false);

private:
    RSA* rsa_public;
    RSA* rsa_private;
};

class SslTool {
public:
    SslTool();
    QByteArray rsaEncode(const QByteArray& buffer);
    QByteArray rsaDecode(const QByteArray& input);

private:
    RSA* rsa_public;
    RSA* rsa_private;
};

#endif // SSLTOOL_H
