# json_parser
this is my joson parser

##Usage
```cpp
	ifstream in(file_name);
	json_parser::sax_parser p(in);
	p.on_object_begin = [](size_t) {cout<<'{';};
	p.on_ojbect_end = [](size_t) {cout<<'}';};
	p.on_array_begin = [](size_t) {cout<<'[';};
	p.on_array_end = [](size_t) {cout<<']';};
	p.on_string_begin = [](size_t) { cout << '"'; };
	p.on_string_end = [](size_t, std::string s) {cout << s << '"';};
	p.on_true_end = [](size_t) { cout << "true"; };
	p.on_false_end = [](size_t) { cout << "false"; };
	p.on_null_end = [](size_t) { cout << "null"; };
	p.on_pair_delimiter = [](size_t) { cout << ':'; };
	p.on_number_end = [](size_t, std::string s) {cout << s;};
	p.on_pair_next = [](size_t) { cout << ','; };
	p.on_array_next = [](size_t) { cout << ','; };
		try
	{
		p.parse();
	}
	catch (json_parser::json_parser_exception& e)
	{
		cout << e.what() << ' ' << e.which() << ':' << e.where() << 
			' ' << e.func() << ':' << e.line_no() << endl;
	}
```

##know issues
###numbers
- leading zeros
- hex
- oct
 
###strings
- tab ,line break or any with backslash

###igore anything after close
