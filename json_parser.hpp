#pragma once

#include <type_traits>
#include <cctype>
#include <string>
#include <list>
#include <functional>
#include <exception>
#include <istream>
#include <sstream>
#include <fstream>

namespace json_parser
{
	enum class json_parser_token
	{
		json,
		brace_begin,
		object = brace_begin,
		brace_end,
		pair,
		pair_key,
		colon,//Ã°ºÅ
		pair_value,
		comma,//¶ººÅ
		bracket_begin,
		Array = bracket_begin,
		bracket_end,
		string,
		number,
		True,
		False,
		null,
	};

	const char* token_string(json_parser_token t)
	{
#define TOKEN_STR(x) #x
		switch (t)
		{
		case json_parser::json_parser_token::json:
			return TOKEN_STR(json);
			break;
		case json_parser::json_parser_token::object:
			return TOKEN_STR(object);
			break;
		case json_parser::json_parser_token::pair:
			return TOKEN_STR(pair);
			break;
		case json_parser::json_parser_token::pair_key:
			return TOKEN_STR(pair_key);
			break;
		case json_parser::json_parser_token::pair_value:
			return TOKEN_STR(pair_value);
			break;
		case json_parser::json_parser_token::colon:
			return TOKEN_STR(colon);
			break;
		case json_parser::json_parser_token::comma:
			return TOKEN_STR(comma);
			break;
		case json_parser::json_parser_token::Array:
			return TOKEN_STR(Array);
			break;
		case json_parser::json_parser_token::string:
			return TOKEN_STR(string);
			break;
		case json_parser::json_parser_token::number:
			return TOKEN_STR(number);
			break;
		case json_parser::json_parser_token::True:
			return TOKEN_STR(True);
			break;
		case json_parser::json_parser_token::False:
			return TOKEN_STR(False);
			break;
		case json_parser::json_parser_token::null:
			return TOKEN_STR(null);
			break;
		default:
			break;
		}
		return "";
#undef TOKEN_STR
	}

	class json_parser_exception
	{
	public:
		json_parser_exception() = delete;
		json_parser_exception(const std::string& s, json_parser_token t, size_t index, const char* file, size_t line): 
			m_s(s), m_t(t), m_n(index), m_sfunc(file), m_lineNo(line) { }
		json_parser_exception(const json_parser_exception& rh): 
			m_s(rh.m_s), m_t(rh.m_t), m_n(rh.m_n), m_sfunc(rh.m_sfunc), m_lineNo(rh.m_lineNo) { }
		~json_parser_exception() = default;

		const char* what() {return m_s.c_str();}
		const char* which() {return token_string(m_t);}
		size_t where() {return m_n;}
		const char* func(){return m_sfunc.c_str();}
		size_t line_no(){return m_lineNo;}

	protected:
		std::string m_s;
		json_parser_token m_t;
		size_t m_n;
		std::string m_sfunc;
		size_t m_lineNo;
	};

	using json_begin_handler = std::function<void(size_t)>;
	using json_end_handler = json_begin_handler;

	using object_begin_handler = std::function<void(size_t)>;
	using object_end_handler = object_begin_handler;

	using pair_begin_handler = object_begin_handler;
	using pair_end_handler = object_end_handler;

	using pair_key_begin_handler = object_begin_handler;
	using pair_key_end_handler = object_end_handler;//std::function<void(size_t, const std::string&)>;

	using pair_delimiter_handler = object_begin_handler;

	using pair_value_begin_handler = object_begin_handler;
	using pair_value_end_handler = object_end_handler;

	using pair_next_handler = object_begin_handler;

	using array_begin_handler = object_begin_handler;
	using array_end_handler = object_end_handler;

	using array_next_handler = object_begin_handler;

	using string_begin_handler = object_begin_handler;
	using string_end_handler = std::function <void(size_t, const std::string&)>;

	using true_begin_handler = object_begin_handler;
	using true_end_handler = object_end_handler;

	using false_begin_handler = object_begin_handler;
	using false_end_handler = object_end_handler;

	using null_begin_handler = object_begin_handler;
	using null_end_handler = object_end_handler;

	using number_begin_handler = object_begin_handler;
	using number_end_handler = string_end_handler;

	class sax_parser
	{
#define PARSER_EXCEPTION(s) json_parser_exception(s, m_stack.back(), pos(), __FUNCTION__, __LINE__)
		typedef std::string::value_type  char_type;
		typedef std::string::size_type size_type;
	public:
		sax_parser(std::istream& stream):m_istream(stream){};
		sax_parser(const sax_parser&) = delete;
		sax_parser(sax_parser&&) = delete;
		~sax_parser() = default;

		int parse()
		{
			m_stack.clear();

			m_stack.push_back(json_parser_token::json);
			if (on_json_begin) on_json_begin(0);

			switch (next_pos())
			{
			case '{':
				return parse_object();
			case '[':
				return parse_array();
			default:
				throw PARSER_EXCEPTION("Unexpected begin, need '{', or '['");
			}

			if (on_json_end) on_json_end(pos());
			m_stack.pop_back();
			return 0;
		}
	protected:
		int parse_object()
		{
			m_stack.push_back(json_parser_token::object);//¿ªÊ¼object
			if (on_object_begin) on_object_begin(pos());

			if ('}' != next_pos())
			{
				while (true)
				{
					parse_pair();

					if (',' != next_pos())
						break;
					else
					{
						if (on_pair_next) on_pair_next(pos());
						next_pos();
					}
				}
			}

			if (!char_in(current_char(), "}"))
				throw PARSER_EXCEPTION("Object should end with '}'");

			if (on_ojbect_end) on_ojbect_end(pos());
			m_stack.pop_back();//½áÊøobject
			return 0;
		}

		int parse_array()
		{
			m_stack.push_back(json_parser_token::Array);
			if (on_array_begin) on_array_begin(pos());

			if (']' != next_pos())
			{
				last_pos();
				while (true)
				{
					parse_value(next_pos());

					if (',' != next_pos())
						break;
					else
					{
						if (on_array_next) on_array_next(pos());
					}					
				}
			}

			if (!char_in(current_char(), "]"))
				throw PARSER_EXCEPTION("Array should end with ']'.");

			if (on_array_end) on_array_end(pos());
			m_stack.pop_back();
			return 0;
		}
		int parse_pair()
		{
			m_stack.push_back(json_parser_token::pair);
			if (on_pair_begin) on_pair_begin(pos());

			m_stack.push_back(json_parser_token::pair_key);
			if (on_pair_key_begin) on_pair_key_begin(pos());

			if (!char_in(current_char(), "\""))
				throw PARSER_EXCEPTION("Pair should start with a string.");

			parse_string();
			if (on_pair_key_end) on_pair_key_end(pos());
			m_stack.pop_back();

			if (!char_in(next_pos(), ":"))
				throw PARSER_EXCEPTION("Pair should delimited with ':'.");
			if (on_pair_delimiter) on_pair_delimiter(pos());

			m_stack.push_back(json_parser_token::pair_value);
			if (on_pair_value_begin) on_pair_value_begin(pos());
			parse_value(next_pos());
			if (on_pair_value_end) on_pair_value_end(pos());
			m_stack.pop_back();

			if (on_pair_end) on_pair_end(pos());
			m_stack.pop_back();
			return 0;
		}

		int parse_value(char_type begin_c)
		{
			switch (begin_c)
			{
			case '{':
				parse_object();
				break;
			case '[':
				parse_array();
				break;
			case '"':
				parse_string();
				break;
			case 't':
				parse_true();
				break;
			case 'f':
				parse_false();
				break;
			case 'n':
				parse_null();
				break;
			default:
				parse_number();
				break;
			}
			return 0;
		}

		int parse_string()
		{
			m_stack.push_back(json_parser_token::string);
			if (on_string_begin) on_string_begin(pos());

			char_type last(current_char());
			std::string s;
			while (true)
			{
				if ('"' == next_char()
					&& ('\\' != last //"*"
					||  (s.size() > 1 && '\\' == *(s.end() - 2))))//"\\"
				{
					break;
				}
				last = current_char();
				s.push_back(last);
			}

			if (on_string_end) on_string_end(pos(), s);
			m_stack.pop_back();
			return 0;
		}

		int parse_true()
		{
			m_stack.push_back(json_parser_token::True);
			if (on_true_begin) on_true_begin(pos());

			if (!parse_bool_null("rue"))//except first char 't'
				throw PARSER_EXCEPTION("Not a literal \"true\"");

			if (on_true_end) on_true_end(pos());
			m_stack.pop_back();
			return 0;
		}

		int parse_false()
		{
			m_stack.push_back(json_parser_token::False);
			if (on_false_begin) on_false_begin(pos());

			if (!parse_bool_null("alse"))//except first char 'f'
				throw PARSER_EXCEPTION("Not a literal \"false\"");

			if (on_false_end) on_false_end(pos());
			m_stack.pop_back();
			return 0;
		}

		int parse_null()
		{
			m_stack.push_back(json_parser_token::null);
			if (on_null_begin) on_null_begin(pos());

			if (!parse_bool_null("ull"))//except first char 'n'
				throw PARSER_EXCEPTION("Not a literal \"null\"");

			if (on_null_end) on_null_end(pos());
			m_stack.pop_back();
			return 0;
		}

		int parse_number()
		{
			m_stack.push_back(json_parser_token::number);
			if (on_number_begin) on_number_begin(pos());

			if (!char_in(current_char(), "+-0123456789"))
				throw PARSER_EXCEPTION("Unexpect starting of a number.");

			std::string number;
			number.push_back(current_char());
			while (char_in(next_char(), "+-0123456789eE."))
				number.push_back(current_char());

			//regex validate

			if (on_number_end) on_number_end(pos(), number);
			m_stack.pop_back();
			last_pos();
			return 0;
		}

		bool parse_bool_null(const std::string& s)
		{
			for (auto c : s)
			{
				if (c != next_char())
				{
					return false;
				}
			}
			return true;
		}

		char_type next_pos()
		{
			while (m_istream.get(m_cCurrent))
			{								
				if (!std::isspace(current_char()))
					return current_char();
			}
			throw PARSER_EXCEPTION("Unexpected end.");
		}

		bool char_in(char_type c, const std::string& cs)
		{
			return cs.find(c) != std::string::npos;
		}

		char_type next_char()
		{
			if (m_istream.get(m_cCurrent))
			{
				return current_char();
			}
			throw PARSER_EXCEPTION("Unexpected end.");
		}

		char_type current_char()
		{
			return m_cCurrent;
		}

		size_type pos()
		{
			return m_istream.tellg();
		}

		void last_pos()
		{
			m_istream.unget();
		}
	private:
		std::istream& m_istream;
		char_type m_cCurrent;
		using token_stack = std::list < json_parser_token > ;
		token_stack m_stack;
	public:
		json_begin_handler on_json_begin = nullptr;
		json_end_handler on_json_end = nullptr;
		object_begin_handler on_object_begin = nullptr;
		object_end_handler on_ojbect_end = nullptr;
		array_begin_handler on_array_begin = nullptr;
		array_end_handler on_array_end = nullptr;
		pair_begin_handler on_pair_begin = nullptr;
		pair_end_handler on_pair_end = nullptr;
		pair_value_begin_handler on_pair_value_begin = nullptr;
		pair_value_end_handler on_pair_value_end = nullptr;
		pair_delimiter_handler on_pair_delimiter = nullptr;
		pair_key_begin_handler on_pair_key_begin = nullptr;
		pair_key_end_handler on_pair_key_end = nullptr;
		string_begin_handler on_string_begin = nullptr;
		string_end_handler on_string_end = nullptr;
		true_begin_handler on_true_begin = nullptr;
		true_end_handler on_true_end = nullptr;
		false_begin_handler on_false_begin = nullptr;
		false_end_handler on_false_end = nullptr;
		null_begin_handler on_null_begin = nullptr;
		null_end_handler on_null_end = nullptr;
		number_begin_handler on_number_begin = nullptr;
		number_end_handler on_number_end = nullptr;
		pair_next_handler on_pair_next = nullptr;
		array_next_handler on_array_next = nullptr;
#undef PARSER_EXCEPTION
	};
};
