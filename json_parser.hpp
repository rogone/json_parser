#pragma once

#include <type_traits>
#include <cctype>
#include <string>
#include <list>
#include <functional>
#include <exception>

namespace json_parser
{
    enum class json_parser_token
    {
        json,
        brace_begin,
        object = brace_begin,
        brace_end,
        object_end = brace_end,
        pair,
        pair_end,
        pair_key,
        pair_key_end,
        colon,//Ã°ºÅ
        pair_value = colon,
        pair_value_end,
        comma,//¶ººÅ
        bracket_begin,
        Array = bracket_begin,
        bracket_end,
        string,
        string_end,
        number,
        number_end,
        True,
        true_end,
        False,
        true_end,
        null,
        null_end,
    };

    const char* token_string(json_parser_token t)
    {
        switch (t)
        {
        case json_parser::json_parser_token::json:
            return #json;
            break;
        case json_parser::json_parser_token::object:
            return #object;
            break;
        case json_parser::json_parser_token::pair:
            return #pair;
            break;
        case json_parser::json_parser_token::pair_key:
            return #pair_key;
            break;
        case json_parser::json_parser_token::pair_key_end:
            break;
        case json_parser::json_parser_token::pair_value:
            break;
        case json_parser::json_parser_token::comma:
            break;
        case json_parser::json_parser_token::Array:
            break;
        case json_parser::json_parser_token::string:
            break;
        case json_parser::json_parser_token::number:
            break;
        case json_parser::json_parser_token::True:
            break;
        case json_parser::json_parser_token::False:
            break;
        case json_parser::json_parser_token::null:
            break;
        default:
            break;
        }
        return "";
    }

    class json_parser_exception
    {
    public:
        json_parser_exception() = delete;
        json_parser_exception(const std::string& s): m_s(s) { }
        json_parser_exception(const json_parser_exception& rh): m_s(rh.m_s) { }
        ~json_parser_exception() = default;

        const char* what() {return m_s.c_str();}
    protected:
        std::string m_s;
    };

    class json_parser_empty_string: public json_parser_exception
    {
    public:
        json_parser_empty_string():json_parser_exception("Empty string") { };
    };

    class json_parser_unexpected_begin: public json_parser_exception
    {
    public:
        json_parser_unexpected_begin(json_parser_token t, size_t index):
            json_parser_exception("Unexpected begin ")
        {
        };
    };

    class json_parser_unexpected_end: public json_parser_exception
    {
    public:
        json_parser_unexpected_end(json_parser_token t):
            json_parser_exception("Unexpected end")
        {
        };
    };

    class json_parser_syntax_error: public json_parser_exception
    {
    public:
        json_parser_syntax_error(json_parser_token t, size_t index):
            json_parser_exception("Syntax error")
        { }
    };

    using json_begin_handler = std::function<void(size_t)>;
    using json_end_handler = json_begin_handler;

    using object_begin_handler = std::function<void(size_t)>;
    using object_end_handler = object_begin_handler;

    using pair_begin_handler = object_begin_handler;
    using pair_end_handler = object_end_handler;

    using pair_key_begin_handler = object_begin_handler;
    using pair_key_end_handler = object_end_handler;//std::function<void(size_t, const std::string&)>;

    using pair_value_begin_handler = object_begin_handler;
    using pair_value_end_handler = object_end_handler;

    using array_begin_handler = object_begin_handler;
    using array_end_handler = object_end_handler;

    using string_begin_handler = object_begin_handler;
    using string_end_handler = std::function <void(size_t, const std::string&)>;

    using true_begin_handler = object_begin_handler;
    using true_end_handler = object_end_handler;

    using false_begin_handler = object_begin_handler;
    using false_end_handler = object_end_handler;

    using null_begin_handler = object_begin_handler;
    using null_end_handler = object_end_handler;

    using number_begin_handler = object_begin_handler;
    using number_end_handler = object_end_handler;


    class sax_parser
    {
        typedef std::string::value_type  char_type;
        typedef std::string::size_type size_type;
    public:
        sax_parser(const std::string& s = std::string()):m_s(s)
        {
        }
        sax_parser() = delete;
        sax_parser(const sax_parser&) = delete;
        ~sax_parser() = default;

        int parse(const std::string& s)
        {
            m_s = s;
            return parse();
        }

        int parse()
        {
            m_stack.clear();
            m_index = std::string::npos;

            m_stack.push_back(json_parser_token::json);
            if (on_json_begin) on_json_begin(0);

            switch (next_pos())
            {
            case '{':
                return parse_object();
            case '[':
                return parse_array();
            default:
                throw json_parser_unexpected_begin(m_stack.back(), m_index);
            }

            if (on_json_end) on_json_end(m_index);
            m_stack.pop_back();
            return 0;
        }
    protected:
        int parse_object()
        {
            m_stack.push_back(json_parser_token::object);//¿ªÊ¼object
            if (on_object_begin) on_object_begin(m_index);

            if ('}' != next_pos())
            {
                while (true)
                {
                    parse_pair();

                    if (',' != next_pos())
                    {
                        break;
                    }
                }
            }

            if (!char_in(current_char(), "}"))
                throw json_parser_syntax_error(m_stack.back(), m_index);

            if (on_ojbect_end) on_ojbect_end(m_index);
            m_stack.pop_back();//½áÊøobject
            return 0;
        }

        int parse_array()
        {
            m_stack.push_back(json_parser_token::Array);
            if (on_array_begin) on_array_begin(m_index);

            if (']' != next_pos())
            {
                while (true)
                {
                    parse_value();

                    if (',' != next_pos())
                    {
                        break;
                    }
                }
            }

            if (!char_in(current_char(), "]"))
                throw json_parser_syntax_error(m_stack.back(), m_index);

            if (on_array_end) on_array_end(m_index);
            m_stack.pop_back();
            return 0;
        }
        int parse_pair()
        {
            m_stack.push_back(json_parser_token::pair);
            if (on_pair_begin) on_pair_begin(m_index);

            m_stack.push_back(json_parser_token::pair_key);
            if (on_pair_key_begin) on_pair_key_begin(m_index);
            size_t start = m_index;
            parse_string();
            size_t end = m_index;
            if (on_pair_key_end) on_pair_key_end(m_index);
            m_stack.pop_back();

            if (!char_in(next_pos(), ":"))
                throw json_parser_syntax_error(m_stack.back(), m_index);

            m_stack.push_back(json_parser_token::pair_value);
            if (on_pair_value_begin) on_pair_value_begin(m_index);
            parse_value();
            if (on_pair_value_end) on_pair_value_end(m_index);
            m_stack.pop_back();

            if (on_pair_end) on_pair_end(m_index);
            m_stack.pop_back();
            return 0;
        }

        int parse_value()
        {
            switch (next_pos())
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
            if (on_string_begin) on_string_begin(m_index);

            auto start = m_index;
            char_type last(current_char());
            std::string s;
            while (true)
            {
                if ('"' == next_char()
                    && '\\' != last)
                {
                    break;
                }
                last = current_char();
                s.push_back(last);
            }

            auto end = m_index;
            if (on_string_end) on_string_end(m_index, s);
            m_stack.pop_back();
            return 0;
        }

        int parse_true()
        {
            m_stack.push_back(json_parser_token::True);
            if (on_true_begin) on_true_begin(m_index);

            if (!parse_bool_null("rue"))//except first char 't'
                throw json_parser_syntax_error(m_stack.back(), m_index);

            if (on_true_end) on_true_end(m_index);
            m_stack.pop_back();
            return 0;
        }

        int parse_false()
        {
            m_stack.push_back(json_parser_token::False);
            if (on_false_begin) on_false_begin(m_index);

            if (!parse_bool_null("alse"))//except first char 'f'
                throw json_parser_syntax_error(m_stack.back(), m_index);

            if (on_false_end) on_false_end(m_index);
            m_stack.pop_back();
            return 0;
        }

        int parse_null()
        {
            m_stack.push_back(json_parser_token::null);
            if (on_null_begin) on_null_begin(m_index);

            if (!parse_bool_null("ull"))//except first char 'n'
                throw json_parser_syntax_error(m_stack.back(), m_index);

            if (on_null_end) on_null_end(m_index);
            m_stack.pop_back();
            return 0;
        }

        int parse_number()
        {
            m_stack.push_back(json_parser_token::number);
            if (on_number_begin) on_number_begin(m_index);

            if (!char_in(current_char(), "+-0123456789"))
                throw json_parser_syntax_error(m_stack.back(), m_index);

            std::string number;
            number.push_back(current_char());
            while (char_in(next_char(), "+-0123456789eE."))
                number.push_back(current_char());

            //regex validate

            if (on_number_end) on_number_end(m_index);
            m_stack.pop_back();
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
            while (++m_index < m_s.size())
            {                                
                if (!std::isspace(current_char()))
                    return current_char();
            }
            throw json_parser_unexpected_end(m_stack.back());
        }

        bool char_in(char_type c, const std::string& cs)
        {
            return cs.find(c) != std::string::npos;
        }

        char_type next_char()
        {
            if (++m_index >= m_s.size())
            {
                throw json_parser_unexpected_end(m_stack.back());
            }
            return current_char();
        }

        char_type current_char()
        {
            return m_s.at(m_index);
        }
    private:
        std::string m_s;
        size_type m_index;
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
    };
};
