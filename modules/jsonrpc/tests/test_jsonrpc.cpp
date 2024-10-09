/**************************************************************************/
/*  test_jsonrpc.cpp                                                      */
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

#include "tests/test_macros.h"
#include "tests/test_utils.h"

#include "test_jsonrpc.h"

#include "core/io/json.h"

#include "../jsonrpc.h"

namespace TestJSONRPC {

void check_error_code(const Dictionary &p_dict, const JSONRPC::ErrorCode &p_code) {
	CHECK(p_dict["jsonrpc"] == "2.0");
	REQUIRE(p_dict.has("error"));
	const Dictionary &err_body = p_dict["error"];
	const int &code = err_body["code"];
	CHECK(code == p_code);
}

void check_invalid(const Dictionary &p_dict) {
	check_error_code(p_dict, JSONRPC::INVALID_REQUEST);
}

void check_invalid_string(const String &p_str) {
	JSON json;
	REQUIRE(json.parse(p_str) == OK);
	const Dictionary &dict = json.get_data();
	check_invalid(dict);
}

void test_process_action(const Variant &p_in, const Variant &p_expected, bool p_process_array_elements = false) {
	TestClassJSONRPC json_rpc = TestClassJSONRPC();
	const Variant &observed = json_rpc.process_action(p_in, p_process_array_elements);
	CHECK(observed == p_expected);
}

void test_process_string(const String &p_in, const String &p_expected) {
	TestClassJSONRPC json_rpc = TestClassJSONRPC();
	const String &out_str = json_rpc.process_string(p_in);
	CHECK(out_str == p_expected);
}

void check_error_no_method(const Dictionary &p_dict) {
	check_error_code(p_dict, JSONRPC::METHOD_NOT_FOUND);
}

void test_process_action_bad_method(const Dictionary &p_in) {
	TestClassJSONRPC json_rpc = TestClassJSONRPC();
	const Dictionary &out_dict = json_rpc.process_action(p_in);
	check_error_no_method(out_dict);
}

TEST_CASE("[JSONRPC] process_action invalid") {
	JSONRPC json_rpc = JSONRPC();

	check_invalid(json_rpc.process_action("String is invalid"));
	check_invalid(json_rpc.process_action(1234));
	check_invalid(json_rpc.process_action(false));
	check_invalid(json_rpc.process_action(3.14159));
}

TEST_CASE("[JSONRPC] process_string invalid") {
	JSONRPC json_rpc = JSONRPC();

	check_invalid_string(json_rpc.process_string("\"String is invalid\""));
	check_invalid_string(json_rpc.process_string("1234"));
	check_invalid_string(json_rpc.process_string("false"));
	check_invalid_string(json_rpc.process_string("3.14159"));
}

TEST_CASE("[JSONRPC] process_action Dictionary") {
	ClassDB::register_class<TestClassJSONRPC>();

	Dictionary in_dict = Dictionary();
	in_dict["method"] = "something";
	in_dict["id"] = "ID";
	in_dict["params"] = "yes";

	Dictionary expected_dict = Dictionary();
	expected_dict["jsonrpc"] = "2.0";
	expected_dict["id"] = "ID";
	expected_dict["result"] = "yes, please";

	test_process_action(in_dict, expected_dict);
}

TEST_CASE("[JSONRPC] process_action Array") {
	Array in;
	Dictionary in_1;
	in_1["method"] = "something";
	in_1["id"] = 1;
	in_1["params"] = "more";
	in.push_back(in_1);
	Dictionary in_2;
	in_2["method"] = "something";
	in_2["id"] = 2;
	in_2["params"] = "yes";
	in.push_back(in_2);

	Array expected;
	Dictionary expected_1;
	expected_1["jsonrpc"] = "2.0";
	expected_1["id"] = 1;
	expected_1["result"] = "more, please";
	expected.push_back(expected_1);
	Dictionary expected_2;
	expected_2["jsonrpc"] = "2.0";
	expected_2["id"] = 2;
	expected_2["result"] = "yes, please";
	expected.push_back(expected_2);

	test_process_action(in, expected, true);
}

TEST_CASE("[JSONRPC] process_string Dictionary") {
	const String in = "{\"method\":\"something\",\"id\":\"ID\",\"params\":\"yes\"}";
	const String expected = "{\"id\":\"ID\",\"jsonrpc\":\"2.0\",\"result\":\"yes, please\"}";

	test_process_string(in, expected);
}

TEST_CASE("[JSONRPC] process_action bad method") {
	Dictionary in_dict;
	in_dict["method"] = "nothing";

	test_process_action_bad_method(in_dict);
}

String TestClassJSONRPC::something(const String &p_in) {
	return p_in + ", please";
}

void TestClassJSONRPC::_bind_methods() {
	ClassDB::bind_method(D_METHOD("something", "in"), &TestClassJSONRPC::something);
}

} // namespace TestJSONRPC
