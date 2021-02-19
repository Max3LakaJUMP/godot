/*************************************************************************/
/*  json.cpp                                                             */
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

#include "red_json.h"

#include "core/print_string.h"
#include "modules/red/red_engine.h"

const char *REDJSON::tk_name[TK_MAX] = {
	"'{'",
	"'}'",
	"'['",
	"']'",
	"identifier",
	"string",
	"number",
	"':'",
	"','",
	"EOF",
};

static String _make_indent(const String &p_indent, int p_size) {

	String indent_text = "";
	if (!p_indent.empty()) {
		for (int i = 0; i < p_size; i++)
			indent_text += p_indent;
	}
	return indent_text;
}

String REDJSON::_print_var(const Variant &p_var, const String &p_indent, int p_cur_indent, bool p_sort_keys, bool globalize_path) {

	String colon = ":";
	String end_statement = "";
	int t = p_var.get_type();

	if (!p_indent.empty()) {
		colon += " ";
		end_statement += "\n";
	}

	switch (t) {
		case Variant::NIL: return "null";
		case Variant::BOOL: return p_var.operator bool() ? "true" : "false";
		case Variant::REAL: return rtos(p_var);
		case Variant::INT: return itos(p_var);
		case Variant::VECTOR2: {
			String s = "{";
			s += end_statement;
			Vector2 var = p_var;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"x\"" + colon + rtos(var.x) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"y\"" + colon + rtos(var.y) + end_statement;
			s += _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		case Variant::VECTOR3: {
			String s = "{";
			s += end_statement;
			Vector3 var = p_var;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"x\"" + colon + rtos(var.x) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"y\"" + colon + rtos(var.y) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"z\"" + colon + rtos(var.z) + end_statement;
			s += _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		case Variant::COLOR: {
			String s = "{";
			s += end_statement;
			Color var = p_var;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"r\"" + colon + rtos(var.r) + "," +end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"g\"" + colon + rtos(var.g) + "," +end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"b\"" + colon + rtos(var.b) + "," +end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"a\"" + colon + rtos(var.a) + end_statement;
			s += _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		//case Variant::REAL: return rtos(p_var);
		//case Variant::POOL_INT_ARRAY:
		case Variant::POOL_REAL_ARRAY:
		//case Variant::POOL_STRING_ARRAY:
		//case Variant::POOL_BYTE_ARRAY:
		case Variant::POOL_VECTOR2_ARRAY:
		//case Variant::POOL_VECTOR3_ARRAY:
		case Variant::POOL_COLOR_ARRAY:
		{	
			String s = "{";
			s += end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"pld\"" + colon;

			s += "[";
			s += end_statement;
			Array a = p_var;
			for (int i = 0; i < a.size(); i++) {
				if (i > 0) {
					s += ",";
					s += end_statement;
				}
				s += _make_indent(p_indent, p_cur_indent + 1) + _print_var(a[i], p_indent, p_cur_indent + 1, p_sort_keys, globalize_path);
			}
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "]";
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		}
		case Variant::TRANSFORM: {
			String s = "{";
			s += end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"d\"" + colon;
			s += "[";
			Transform a = p_var;
			s += rtos(a.basis[0].x) + ", ";
			s += rtos(a.basis[0].y) + ", ";
			s += rtos(a.basis[0].z) + ", ";
			s += rtos(a.basis[1].x) + ", ";
			s += rtos(a.basis[1].y) + ", ";
			s += rtos(a.basis[1].z) + ", ";
			s += rtos(a.basis[2].x) + ", ";
			s += rtos(a.basis[2].y) + ", ";
			s += rtos(a.basis[2].z) + ", ";
			s += rtos(a.origin.x) + ", ";
			s += rtos(a.origin.y) + ", ";
			s += rtos(a.origin.z);
			s += "]";
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		case Variant::TRANSFORM2D: {
			String s = "{";
			s += end_statement;
			Color var = p_var;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"t\"" + colon + _print_var(t, p_indent, p_cur_indent + 1, p_sort_keys) + "," + end_statement;
			s += _make_indent(p_indent, p_cur_indent + 1) + "\"d\"" + colon;
			s += "[";
			Transform2D a = p_var;
			s += rtos(a[0].x) + ", ";
			s += rtos(a[0].y) + ", ";
			s += rtos(a[1].x) + ", ";
			s += rtos(a[1].y) + ", ";
			s += rtos(a[2].x) + ", ";
			s += rtos(a[2].y);
			s += "]";
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		case Variant::ARRAY: {

			String s = "[";
			s += end_statement;
			Array a = p_var;
			for (int i = 0; i < a.size(); i++) {
				if (i > 0) {
					s += ",";
					s += end_statement;
				}
				s += _make_indent(p_indent, p_cur_indent + 1) + _print_var(a[i], p_indent, p_cur_indent + 1, p_sort_keys, globalize_path);
			}
			s += end_statement + _make_indent(p_indent, p_cur_indent) + "]";
			return s;
		};
		case Variant::DICTIONARY: {

			String s = "{";
			s += end_statement;
			Dictionary d = p_var;
			List<Variant> keys;
			d.get_key_list(&keys);

			if (p_sort_keys)
				keys.sort();

			for (List<Variant>::Element *E = keys.front(); E; E = E->next()) {

				if (E != keys.front()) {
					s += ",";
					s += end_statement;
				}
				s += _make_indent(p_indent, p_cur_indent + 1) + _print_var(String(E->get()), p_indent, p_cur_indent + 1, p_sort_keys);
				s += colon;
				s += _print_var(d[E->get()], p_indent, p_cur_indent + 1, p_sort_keys, globalize_path);
			}

			s += end_statement + _make_indent(p_indent, p_cur_indent) + "}";
			return s;
		};
		case Variant::STRING: {
			if (globalize_path){
				String var = p_var;
				int pos = var.find("res://");
				if (pos!=-1)
					return "\"" + red::globalize(var).json_escape() + "\"";
			}
			return "\"" + String(p_var).json_escape() + "\"";
		};
		default: return "\"" + String(p_var).json_escape() + "\"";
	}
}

String REDJSON::print(const Variant &p_var, const String &p_indent, bool p_sort_keys, bool globalize_path) {

	return _print_var(p_var, p_indent, 0, p_sort_keys, globalize_path);
}

Error REDJSON::_get_token(const CharType *p_str, int &index, int p_len, Token &r_token, int &line, String &r_err_str) {

	while (p_len > 0) {
		switch (p_str[index]) {

			case '\n': {

				line++;
				index++;
				break;
			};
			case 0: {
				r_token.type = TK_EOF;
				return OK;
			} break;
			case '{': {

				r_token.type = TK_CURLY_BRACKET_OPEN;
				index++;
				return OK;
			};
			case '}': {

				r_token.type = TK_CURLY_BRACKET_CLOSE;
				index++;
				return OK;
			};
			case '[': {

				r_token.type = TK_BRACKET_OPEN;
				index++;
				return OK;
			};
			case ']': {

				r_token.type = TK_BRACKET_CLOSE;
				index++;
				return OK;
			};
			case ':': {

				r_token.type = TK_COLON;
				index++;
				return OK;
			};
			case ',': {

				r_token.type = TK_COMMA;
				index++;
				return OK;
			};
			case '"': {

				index++;
				String str;
				while (true) {
					if (p_str[index] == 0) {
						r_err_str = "Unterminated String";
						return ERR_PARSE_ERROR;
					} else if (p_str[index] == '"') {
						index++;
						break;
					} else if (p_str[index] == '\\') {
						//escaped characters...
						index++;
						CharType next = p_str[index];
						if (next == 0) {
							r_err_str = "Unterminated String";
							return ERR_PARSE_ERROR;
						}
						CharType res = 0;

						switch (next) {

							case 'b': res = 8; break;
							case 't': res = 9; break;
							case 'n': res = 10; break;
							case 'f': res = 12; break;
							case 'r': res = 13; break;
							case 'u': {
								//hexnumbarh - oct is deprecated

								for (int j = 0; j < 4; j++) {
									CharType c = p_str[index + j + 1];
									if (c == 0) {
										r_err_str = "Unterminated String";
										return ERR_PARSE_ERROR;
									}
									if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {

										r_err_str = "Malformed hex constant in string";
										return ERR_PARSE_ERROR;
									}
									CharType v;
									if (c >= '0' && c <= '9') {
										v = c - '0';
									} else if (c >= 'a' && c <= 'f') {
										v = c - 'a';
										v += 10;
									} else if (c >= 'A' && c <= 'F') {
										v = c - 'A';
										v += 10;
									} else {
										ERR_PRINT("BUG");
										v = 0;
									}

									res <<= 4;
									res |= v;
								}
								index += 4; //will add at the end anyway

							} break;
							//case '\"': res='\"'; break;
							//case '\\': res='\\'; break;
							//case '/': res='/'; break;
							default: {
								res = next;
								//r_err_str="Invalid escape sequence";
								//return ERR_PARSE_ERROR;
							} break;
						}

						str += res;

					} else {
						if (p_str[index] == '\n')
							line++;
						str += p_str[index];
					}
					index++;
				}

				r_token.type = TK_STRING;
				r_token.value = str;
				return OK;

			} break;
			default: {

				if (p_str[index] <= 32) {
					index++;
					break;
				}

				if (p_str[index] == '-' || (p_str[index] >= '0' && p_str[index] <= '9')) {
					//a number
					const CharType *rptr;
					double number = String::to_double(&p_str[index], &rptr);
					index += (rptr - &p_str[index]);
					r_token.type = TK_NUMBER;
					r_token.value = number;
					return OK;

				} else if ((p_str[index] >= 'A' && p_str[index] <= 'Z') || (p_str[index] >= 'a' && p_str[index] <= 'z')) {

					String id;

					while ((p_str[index] >= 'A' && p_str[index] <= 'Z') || (p_str[index] >= 'a' && p_str[index] <= 'z')) {

						id += p_str[index];
						index++;
					}

					r_token.type = TK_IDENTIFIER;
					r_token.value = id;
					return OK;
				} else {
					r_err_str = "Unexpected character.";
					return ERR_PARSE_ERROR;
				}
			}
		}
	}

	return ERR_PARSE_ERROR;
}

Error REDJSON::_parse_value(Variant &value, Token &token, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str) {

	if (token.type == TK_CURLY_BRACKET_OPEN) {

		Dictionary d;
		Error err = _parse_object(d, p_str, index, p_len, line, r_err_str);
		if (err)
			return err;
		
		if (d.has("t")){
			int t = d["t"];
			switch (t) { 
				case Variant::VECTOR2: {
					value = Vector2(d["x"], d["y"]);
					return OK;
				};
				case Variant::VECTOR3: {
					value = Vector3(d["x"], d["y"], d["z"]);
					return OK;
				};
				case Variant::COLOR: {
					value = Color(d["r"], d["g"], d["b"], d["a"]);
					return OK;
				};		
				case Variant::POOL_VECTOR2_ARRAY: {
					Array a = d["pld"];
					value = red::pool_vector2_array(a);
					return OK;
				};		
				case Variant::POOL_COLOR_ARRAY: {
					Array a = d["pld"];
					value = red::pool_color_array(a);
					return OK;
				};		
				case Variant::POOL_REAL_ARRAY: {
					Array a = d["pld"];
					value = red::pool_real_array(a);
					return OK;
				};	
				case Variant::TRANSFORM: {
					Array a = d["d"];
					if(a.size() >= 12)
						value = Transform(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11]);
					else
						value = Transform();
					return OK;
				};		
				case Variant::TRANSFORM2D: {
					Array a = d["d"];
					if(a.size() >= 6)
						value = Transform2D(a[0], a[1], a[2], a[3], a[4], a[5]);
					else
						value = Transform2D();
					return OK;
				};	
				default: break;
			}
		}
		
		value = d;
		return OK;
	} else if (token.type == TK_BRACKET_OPEN) {

		Array a;
		Error err = _parse_array(a, p_str, index, p_len, line, r_err_str);
		if (err)
			return err;
		value = a;
		return OK;

	} else if (token.type == TK_IDENTIFIER) {

		String id = token.value;
		if (id == "true")
			value = true;
		else if (id == "false")
			value = false;
		else if (id == "null")
			value = Variant();
		else {
			r_err_str = "Expected 'true','false' or 'null', got '" + id + "'.";
			return ERR_PARSE_ERROR;
		}
		return OK;

	} else if (token.type == TK_NUMBER) {

		value = token.value;
		return OK;
	} else if (token.type == TK_STRING) {

		value = token.value;
		return OK;
	} else {
		r_err_str = "Expected value, got " + String(tk_name[token.type]) + ".";
		return ERR_PARSE_ERROR;
	}
}

Error REDJSON::_parse_array(Array &array, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str) {

	Token token;
	bool need_comma = false;

	while (index < p_len) {

		Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
		if (err != OK)
			return err;

		if (token.type == TK_BRACKET_CLOSE) {

			return OK;
		}

		if (need_comma) {

			if (token.type != TK_COMMA) {

				r_err_str = "Expected ','";
				return ERR_PARSE_ERROR;
			} else {
				need_comma = false;
				continue;
			}
		}

		Variant v;
		err = _parse_value(v, token, p_str, index, p_len, line, r_err_str);
		if (err)
			return err;

		array.push_back(v);
		need_comma = true;
	}

	return ERR_PARSE_ERROR;
}

Error REDJSON::_parse_object(Dictionary &object, const CharType *p_str, int &index, int p_len, int &line, String &r_err_str) {

	bool at_key = true;
	String key;
	Token token;
	bool need_comma = false;

	while (index < p_len) {

		if (at_key) {

			Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK)
				return err;

			if (token.type == TK_CURLY_BRACKET_CLOSE) {

				return OK;
			}

			if (need_comma) {

				if (token.type != TK_COMMA) {

					r_err_str = "Expected '}' or ','";
					return ERR_PARSE_ERROR;
				} else {
					need_comma = false;
					continue;
				}
			}

			if (token.type != TK_STRING) {

				r_err_str = "Expected key";
				return ERR_PARSE_ERROR;
			}

			key = token.value;
			err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK)
				return err;
			if (token.type != TK_COLON) {

				r_err_str = "Expected ':'";
				return ERR_PARSE_ERROR;
			}
			at_key = false;
		} else {

			Error err = _get_token(p_str, index, p_len, token, line, r_err_str);
			if (err != OK)
				return err;

			Variant v;
			err = _parse_value(v, token, p_str, index, p_len, line, r_err_str);
			if (err)
				return err;
			object[key] = v;
			need_comma = true;
			at_key = true;
		}
	}

	return ERR_PARSE_ERROR;
}

Error REDJSON::parse(const String &p_json, Variant &r_ret, String &r_err_str, int &r_err_line) {

	const CharType *str = p_json.ptr();
	int idx = 0;
	int len = p_json.length();
	Token token;
	r_err_line = 0;
	String aux_key;

	Error err = _get_token(str, idx, len, token, r_err_line, r_err_str);
	if (err)
		return err;

	err = _parse_value(r_ret, token, str, idx, len, r_err_line, r_err_str);

	return err;
}
