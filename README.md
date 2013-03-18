WML (Whitespace Markup Language)
================================

Example
-------

	key:
		subKey valueA "\tvalueB" 'value C'

		rawText::
			some raw text
			indentation gets stripped

		"another sub key":
			emptyKey


Library
-------
* written in C++
* 5 files: less than 1000 lines in total
* additional unit tests written using [googletest](https://code.google.com/p/googletest/)
* [boost](http://www.boost.org/) is required for boost::format and boost::lexical_cast
