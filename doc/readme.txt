'Whitespace Markup Language':

	Aims::
		* very simple
		* no clutter while writing
		* only indentation counts
		* empty lines have no meaning
		* embedding text is easy
		* everything is a map


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
