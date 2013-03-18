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
* parser and emitter written in C++
* just 5 header files: less than 1000 lines in total
* additional unit tests written using [googletest](https://code.google.com/p/googletest/)
* [boost](http://www.boost.org/) is required for boost::format and boost::lexical_cast

API example
-----------

### Basics

```c++
const char *text = "keyA valueA\nkeyB valueB";

Node root;

ASSERT_TRUE( root.empty() );

root = parse( text );

ASSERT_FALSE( root.empty() );

int numRootNodes = root.size();
ASSERT_EQ( 2, numRootNodes );

auto node0Key = root[0].key();
auto node0Value = root[0].value();
ASSERT_EQ( "keyA", node0Key );
ASSERT_EQ( "valueA", node0Value );

auto node1Key = root[1].key();
auto node1Value = root[1].value();
ASSERT_EQ( "keyB", node1Key );
ASSERT_EQ( "valueB", node1Value );

// we can also set key and value
root[0].key() = "keyAA";
root[0].value() = "valueAA";
root[1].key() = "keyBB";
root[1].value() = "valueBB";
```

### Indexing by keys

```c++
const char *text = "keyA valueA\nkeyB valueB";

Node root = parse( text );

auto nodeA = root["keyA"];
ASSERT_EQ( "keyA", nodeA.key() );
ASSERT_EQ( "valueA", nodeA.value() );

auto nodeB = root["keyB"];
ASSERT_EQ( "keyB", nodeB.key() );
ASSERT_EQ( "valueB", nodeB.value() );
```

### Conversion using get and as
	
```c++
const char *text = "pie 413\npi 3.1415";

Node root = parse( text );

int pieValue = root[ "pie" ].as<int>();
ASSERT_EQ( 413, pieValue );

float piValue = root.get<float>( "pi" );
ASSERT_FLOAT_EQ( 3.1415, piValue );
```


Longer WML example (docs/readme.txt)
--------------------------------

	'Whitespace Markup Language':

		Aims::
			* very simple
			* no clutter while writing
			* only indentation counts
			* empty lines have no meaning
			* embedding text is easy
			* everything is a map internally


		Example:

			title 		"test\t\t1"
			path		'c:\unescaped.txt'
			version 	1

			content::
				unformated text

				newlines count here

			properties:
				time-changed	10:47am
				flags	archive system hidden

			streams:
				stream:
					data::
						some data

						this is nested too
					flags:
						read
						write
						execute

				stream:
					data::
						key names
						dont have to
						be unique (see stream)
					flags:
						read
						write:
							users	andreas root



		Grammar::
			INDENT, DEINDENT are virtual tokens that control the indentation level
			NEWLINE is a line break

			Indentation is done with tabs only at the moment.

			Here is a rough EBNF syntax for WML:

			root: map

			value: identifier | unescaped_string | escaped_string

			identifier: (!whitespace)+
			unescaped_string: '\'' (!'\'')* '\''
			escaped_string: '"' (!"'")* '"' with support for \t, \n, \\, \', and \"

			key: value

			map: map_entry*

			map_entry: inline_entry | block_entry

			inline_entry: key value+ NEWLINE
			block_entry: key ':' ( ':' NEWLINE INDENT textblock DEINDENT | NEWLINE INDENT non-empty map DEINDENT )

		Note::
			This file is itself a WML file and root["Whitespace Markup Language"]["Example"].data() is the example WML node


