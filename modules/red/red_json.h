/*************************************************************************/
/*  json.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RED_JSON_H
#define RED_JSON_H

#include "core/variant.h"

class REDJSON {

	enum TokenType {
		TK_CURLY_BRACKET_OPEN,
		TK_CURLY_BRACKET_CLOSE,
		TK_BRACKET_OPEN,
		TK_BRACKET_CLOSE,
		TK_IDENTIFIER,
		TK_STRING,
		TK_NUMBER,
		TK_COLON,
		TK_COMMA,
		TK_EOF,
		TK_MAX
	};

	enum PoolType {
		TK_ARRAY,
		TK_POOL_VECTOR2_ARRAY
	};

	enum Expecting {

		EXPECT_OBJECT,
		EXPECT_OBJECT_KEY,
		EXPECT_COLON,
		EXPECT_OBJECT_VALUE,
	};

	struct Token {
		TokenType type;
		Variant value;
	};

	struct DoubleToken {
		TokenType type;
		float value;
	};

	static const char *tk_name[TK_MAX];

	static String _print_var(const Variant &p_var, const String &p_indent, int p_cur_indent, bool p_sort_keys, bool globalize_path = false);

	static Error _get_token(const CharType *p_str, int &index, int p_len, Token &r_token, int &line, String &r_err_str);
	static Error _parse_value(Variant &value, Token &token, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_array(Array &array, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_object(Dictionary &object, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_pool_int(PoolVector<int> &pool, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_pool_real(PoolRealArray &pool, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_pool_vector2(PoolVector<Vector2> &array, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_pool_color(PoolVector<Color> &pool, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str);
	static Error _parse_double(const CharType *p_str, int &index, int p_len, DoubleToken &r_token, int &line, String &r_err_str);
public:
	static String print(const Variant &p_var, const String &p_indent = "", bool p_sort_keys = true, bool globalize_path = false);
	static Error parse(const String &p_json, Variant &r_ret, String &r_err_str, int &r_err_line);
};

#endif // RED_JSON_H