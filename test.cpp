#include <string>
#include "digest.hpp"
#include "mime-base64.hpp"
#include "pbkdf2-sha256.hpp"
#include "taptests.hpp"

void
test_md5 (test::simple& t)
{
    digest::MD5 md5;
    t.ok (md5.hexdigest ()
        == "d41d8cd98f00b204e9800998ecf8427e", "md5 empty data");
    t.ok (md5.add ("abc").hexdigest ()
        == "900150983cd24fb0d6963f7d28e17f72", "md5 a");
    t.ok (md5.add ("abcdbcdecdefdefgefghfghighijhijkijkl").hexdigest ()
        == "1abfcd9645d94ffc9f14286a365988d6", "md5 b");
    t.ok (md5.add ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
             .hexdigest ()
        == "8215ef0796a20bcaaae116d3876c664a", "md5 c");

    digest::base& h = md5;
    h.add ("abc");
    t.ok (h.hexdigest ()
        == "900150983cd24fb0d6963f7d28e17f72", "md5 d");
}

void
test_sha256 (test::simple& t)
{
    digest::SHA256 sha256;
    t.ok (sha256.add ("").hexdigest ()
        == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "sha256 empty data");

    t.ok (sha256.add ("The quick brown fox jumps over the lazy dog.").hexdigest ()
        == "ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c",
        "sha256 quick brown...");

    digest::base& h = sha256;
    h.add ("The quick brown fox jumps over the lazy dog.");
    t.ok (h.hexdigest ()
        == "ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c",
        "sha256 c");
    h.reset ();
    h.add ("The quick brown fox ");
    h.add ("jumps over the lazy dog.");
    t.ok (h.hexdigest ()
        == "ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c",
        "sha256 d");
}

void
test_sha256_more (test::simple& t)
{
    // Authorization: Digest algorithm=SHA-256,qop=auth,...
    digest::SHA256 h1;
    digest::SHA256 h2;
    digest::SHA256 hr;
    std::string a1     = "Mufasa:http-auth@example.org:Circle of Life";
    std::string a2     = "GET:/dir/index.html";
    std::string nonce  = "7ypf/xlj9XXwfDPEoM4URrv/xwf94BcCAzFZH4GiTo0v";
    std::string nc     = "00000001";
    std::string cnonce = "f2/wE4q74E6zIJEtWaHKaf5wv/H5QzzpXusqGemxURZJ";
    std::string got = hr.add (h1.add (a1).hexdigest ())
                        .add (":").add (nonce)
                        .add (":").add (nc)
                        .add (":").add (cnonce)
                        .add (":auth:").add (h2.add (a2).hexdigest ())
                        .hexdigest ();
    t.ok (got == "753927fa0e85d155564e2e272a28d1802ca10daf4496794697cf8db5856cb6c1",
        "http digest authorization");
}

void
test_hmac_sha256 (test::simple& t)
{
    digest::HMAC<digest::SHA256> hmac1 (
        "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b");
    t.ok (hmac1.add ("Hi There").hexdigest ()
        == "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",
        "hmac-sha256 rfc 4231 test case 1");

    digest::HMAC<digest::SHA256> hmac2 ("Jefe");
    t.ok (hmac2.add ("what do ya want for nothing?").hexdigest ()
        == "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843",
        "hmac-sha256 rfc 4231 test case 2");

    digest::HMAC<digest::SHA256> hmac3 (
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa");
    t.ok (hmac3.add ("Test Using Larger Than Block-Size Key - Hash Key First")
               .hexdigest ()
        == "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54",
        "hmac-sha256 rfc 4231 test case 6");

    digest::base& h = hmac3;
    t.ok (h.add ("This is a test using a larger than block-size ke"
                 "y and a larger than block-size data. The key nee"
                 "ds to be hashed before being used by the HMAC al"
                 "gorithm.")
               .hexdigest ()
        == "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2",
        "hmac-sha256 rfc 4231 test case 7");
}

void
test_encode_base64 (test::simple& t)
{
    static const std::basic_string<uint8_t> xinput {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb,
        0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4,
        0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52,
        0xb8, 0x55, 0xef, 0x53, 0x7f, 0x25, 0xc8, 0x95, 0xbf, 0xa7,
        0x82, 0x52, 0x65, 0x29, 0xa9, 0xb6, 0x3d, 0x97, 0xaa, 0x63,
        0x15, 0x64, 0xd5, 0xd7, 0x89, 0xc2, 0xb7, 0x65, 0x44, 0x8c,
        0x86, 0x35, 0xfb, 0x6c, 0x75, 0x39, 0x27, 0xfa, 0x0e, 0x85,
        0xd1, 0x55, 0x56, 0x4e, 0x2e, 0x27, 0x2a, 0x28, 0xd1, 0x80,
        0x2c, 0xa1, 0x0d, 0xaf, 0x44, 0x96, 0x79, 0x46, 0x97, 0xcf,
        0x8d, 0xb5, 0x85, 0x6c, 0xb6, 0xc1, 0xb0, 0x34, 0x4c, 0x61,
        0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b,
        0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7};
    std::string octets (xinput.cbegin (), xinput.cend ());

    static const std::string expected1 =
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3\n"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd\n"
        "wgDJgz2nJuk3bC4yz/c=\n";
    t.ok (mime::encode_base64 (octets) == expected1, "encode_base64/1");

    static const std::string expected2 =
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz/c=";
    t.ok (mime::encode_base64 (octets, "") == expected2, "encode_base64/2");

    static const std::string expected3 =
        "47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW_p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J_oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK_OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz_c=";
    t.ok (mime::encode_base64url (octets) == expected3, "encode_base64url");

    static const std::string expected4 =
        "47DEQpj8HBSa./TImW.5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz/c";
    t.ok (mime::encode_base64crypt (octets) == expected4, "encode_base64crypt");

    octets.clear ();
    for (std::size_t i = 0; i < 54; ++i)
        octets.push_back (i + 40);
    static const std::string expected5 =
        "KCkqKywtLi8wMTIzNDU2Nzg5Ojs8PT4/QEFCQ0RF"
        "RkdISUpLTE1OT1BRUlNUVVZXWFlaW1xd\n";
    t.ok (mime::encode_base64 (octets) == expected5, "encode_base64/1 72 column");

    for (std::size_t i = 54; i < 57; ++i)
        octets.push_back (i + 40);
    static const std::string expected6 =
        "KCkqKywtLi8wMTIzNDU2Nzg5Ojs8PT4/QEFCQ0RF"
        "RkdISUpLTE1OT1BRUlNUVVZXWFlaW1xdXl9g\n";
    t.ok (mime::encode_base64 (octets) == expected6, "encode_base64/1 76 column");

    for (std::size_t i = 57; i < 60; ++i)
        octets.push_back (i + 40);
    static const std::string expected7 =
        "KCkqKywtLi8wMTIzNDU2Nzg5Ojs8PT4/QEFCQ0RF"
        "RkdISUpLTE1OT1BRUlNUVVZXWFlaW1xdXl9g\n"
        "YWJj\n";
    t.ok (mime::encode_base64 (octets) == expected7, "encode_base64/1 80 column");
}

void
test_decode_base64 (test::simple& t)
{
    static const std::basic_string<uint8_t> xinput {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb,
        0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4,
        0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52,
        0xb8, 0x55, 0xef, 0x53, 0x7f, 0x25, 0xc8, 0x95, 0xbf, 0xa7,
        0x82, 0x52, 0x65, 0x29, 0xa9, 0xb6, 0x3d, 0x97, 0xaa, 0x63,
        0x15, 0x64, 0xd5, 0xd7, 0x89, 0xc2, 0xb7, 0x65, 0x44, 0x8c,
        0x86, 0x35, 0xfb, 0x6c, 0x75, 0x39, 0x27, 0xfa, 0x0e, 0x85,
        0xd1, 0x55, 0x56, 0x4e, 0x2e, 0x27, 0x2a, 0x28, 0xd1, 0x80,
        0x2c, 0xa1, 0x0d, 0xaf, 0x44, 0x96, 0x79, 0x46, 0x97, 0xcf,
        0x8d, 0xb5, 0x85, 0x6c, 0xb6, 0xc1, 0xb0, 0x34, 0x4c, 0x61,
        0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b,
        0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7};
    std::string expected (xinput.cbegin (), xinput.cend ());
    std::string got;

    static const std::string b64_1 =
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3\n"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd\n"
        "wgDJgz2nJuk3bC4yz/c=\n";
    t.ok (mime::decode_base64 (b64_1, got), "decode_base64 wrap");
    t.ok (got == expected, "decode_base64 wrap got");

    static const std::string b64_2 =
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz/c=";
    t.ok (mime::decode_base64 (b64_2, got), "decode_base64");
    t.ok (got == expected, "decode_base64 got");

    static const std::string b64_3 =
        "47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW_p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J_oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK_OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz_c=";
    t.ok (mime::decode_base64url (b64_3, got), "decode_base64url");
    t.ok (got == expected, "decode_base64url got");

    static const std::string b64_4 =
        "47DEQpj8HBSa./TImW.5JCeuQeRkm5NMpJWZG3hS"
        "uFXvU38lyJW/p4JSZSmptj2XqmMVZNXXicK3"
        "ZUSMhjX7bHU5J/oOhdFVVk4uJyoo0YAsoQ2vRJZ5"
        "RpfPjbWFbLbBsDRMYdjbOFNcqK/OrwvxK4gd"
        "wgDJgz2nJuk3bC4yz/c";
    t.ok (mime::decode_base64crypt (b64_4, got), "decode_base64crypt");
    t.ok (got == expected, "decode_base64crypt got");
}

void
test_base64_more (test::simple& t)
{
    std::string plain = "Aladdin:open sesame";
    std::string b64 = "QWxhZGRpbjpvcGVuIHNlc2FtZQ==";
    t.ok (mime::encode_base64 (plain, "") == b64, "http basic www-authenticate");
    std::string got;
    mime::decode_base64 (b64, got);
    t.ok (got == plain, "http basic authorization");
}

void
test_pbkdf2_sha256 (test::simple& t)
{
    // https://pythonhosted.org/passlib/lib/passlib.hash.pbkdf2_digest.html

    static const std::basic_string<uint8_t> xsalt {
        0xd1, 0x9a, 0xf3, 0x5e, 0x2b, 0x45, 0x48, 0x69, 0x6d, 0x4d,
        0x09, 0xc1, 0x58, 0xeb, 0x1d, 0x03};
    std::string salt (xsalt.cbegin (), xsalt.cend ());

    t.ok (mime::encode_base64crypt (salt) == "0ZrzXitFSGltTQnBWOsdAw",
        "pbkdf2-sha256 salt");

    std::string got = pbkdf2_sha256::encrypt ("password", salt);
    std::string expected
        = "$pbkdf2-sha256$6400$0ZrzXitFSGltTQnBWOsdAw$"
          "Y11AchqV4b0sUisdZd0Xr97KWoymNE0LNNrnEgY4H9M";
    t.ok (got == expected, "pbkdf2-sha256 encrypt salt");
    t.ok (pbkdf2_sha256::verify ("password", got), "pbkdf2-sha256 verify");
}

int
main ()
{
    test::simple t (34);
    test_md5 (t);
    test_sha256 (t);
    test_sha256_more (t);
    test_hmac_sha256 (t);
    test_encode_base64 (t);
    test_decode_base64 (t);
    test_base64_more (t);
    test_pbkdf2_sha256 (t);
    return t.done_testing ();
}
