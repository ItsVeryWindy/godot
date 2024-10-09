/**************************************************************************/
/*  test_crypto_mbedtls.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "../crypto_mbedtls.h"

#include "tests/test_macros.h"
#include "tests/test_utils.h"

#include "core/crypto/crypto.h"
#include "core/crypto/hashing_context.h"

#include "tests/test_macros.h"
#include "tests/test_utils.h"

namespace TestCryptoMbedTLS {

void hmac_digest_test(HashingContext::HashType ht, String expected_hex) {
	CryptoMbedTLS crypto;
	PackedByteArray key = String("supersecretkey").to_utf8_buffer();
	PackedByteArray msg = String("Return of the MAC!").to_utf8_buffer();
	PackedByteArray digest = crypto.hmac_digest(ht, key, msg);
	String hex = String::hex_encode_buffer(digest.ptr(), digest.size());
	CHECK(hex == expected_hex);
}

void hmac_context_digest_test(HashingContext::HashType ht, String expected_hex) {
	HMACContextMbedTLS ctx;
	PackedByteArray key = String("supersecretkey").to_utf8_buffer();
	PackedByteArray msg1 = String("Return of ").to_utf8_buffer();
	PackedByteArray msg2 = String("the MAC!").to_utf8_buffer();
	Error err = ctx.start(ht, key);
	CHECK(err == OK);
	err = ctx.update(msg1);
	CHECK(err == OK);
	err = ctx.update(msg2);
	CHECK(err == OK);
	PackedByteArray digest = ctx.finish();
	String hex = String::hex_encode_buffer(digest.ptr(), digest.size());
	CHECK(hex == expected_hex);
}

Ref<CryptoKey> create_crypto_key(const String &p_key_path, bool p_public_only) {
	Ref<CryptoKey> crypto_key = Ref<CryptoKey>(CryptoKey::create());
	crypto_key->load(p_key_path, p_public_only);
	return crypto_key;
}

String read_file_s(const String &p_file_path) {
	Ref<FileAccess> file_access = FileAccess::open(p_file_path, FileAccess::READ);
	REQUIRE(file_access.is_valid());
	return file_access->get_as_utf8_string();
}

bool files_equal(const String &p_in_path, const String &p_out_path) {
	const String s_in = read_file_s(p_in_path);
	const String s_out = read_file_s(p_out_path);
	return s_in == s_out;
}

void crypto_key_public_only_test(const String &p_key_path, bool p_public_only) {
	Ref<CryptoKey> crypto_key = create_crypto_key(p_key_path, p_public_only);
	bool is_equal = crypto_key->is_public_only() == p_public_only;
	CHECK(is_equal);
}

void crypto_key_save_test(const String &p_in_path, const String &p_out_path, bool p_public_only) {
	Ref<CryptoKey> crypto_key = create_crypto_key(p_in_path, p_public_only);
	crypto_key->save(p_out_path, p_public_only);
	bool is_equal = files_equal(p_in_path, p_out_path);
	CHECK(is_equal);
}

void crypto_key_save_public_only_test(const String &p_in_priv_path, const String &p_in_pub_path, const String &p_out_path) {
	Ref<CryptoKey> crypto_key = create_crypto_key(p_in_priv_path, false);
	crypto_key->save(p_out_path, true);
	bool is_equal = files_equal(p_in_pub_path, p_out_path);
	CHECK(is_equal);
}

TEST_CASE("[CryptoMbedTLS] HMAC digest") {
	// SHA-256
	hmac_digest_test(HashingContext::HashType::HASH_SHA256, "fe442023f8a7d36a810e1e7cd8a8e2816457f350a008fbf638296afa12085e59");

	// SHA-1
	hmac_digest_test(HashingContext::HashType::HASH_SHA1, "a0ac4cd68a2f4812c355983d94e8d025afe7dddf");
}

TEST_CASE("[HMACContext] HMAC digest") {
	// SHA-256
	hmac_context_digest_test(HashingContext::HashType::HASH_SHA256, "fe442023f8a7d36a810e1e7cd8a8e2816457f350a008fbf638296afa12085e59");

	// SHA-1
	hmac_context_digest_test(HashingContext::HashType::HASH_SHA1, "a0ac4cd68a2f4812c355983d94e8d025afe7dddf");
}

TEST_CASE("[Crypto] CryptoKey is_public_only") {
	crypto_key_public_only_test(TestUtils::get_data_path("crypto/in.key"), false);
	crypto_key_public_only_test(TestUtils::get_data_path("crypto/in.pub"), true);
}

TEST_CASE("[Crypto] CryptoKey save") {
	const String in_priv_path = TestUtils::get_data_path("crypto/in.key");
	const String out_priv_path = TestUtils::get_data_path("crypto/out.key");
	crypto_key_save_test(in_priv_path, out_priv_path, false);

	const String in_pub_path = TestUtils::get_data_path("crypto/in.pub");
	const String out_pub_path = TestUtils::get_data_path("crypto/out.pub");
	crypto_key_save_test(in_pub_path, out_pub_path, true);
}

TEST_CASE("[Crypto] CryptoKey save public_only") {
	const String in_priv_path = TestUtils::get_data_path("crypto/in.key");
	const String in_pub_path = TestUtils::get_data_path("crypto/in.pub");
	const String out_path = TestUtils::get_data_path("crypto/out_public_only.pub");
	crypto_key_save_public_only_test(in_priv_path, in_pub_path, out_path);
}
} // namespace TestCryptoMbedTLS
