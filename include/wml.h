/*
Copyright 2013 Andreas Kirsch

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "wml_node.h"
#include "wml_detail_parser.h"
#include "wml_detail_emitter.h"

namespace wml {
	inline Node parse( const std::string &content, const std::string &sourceIdentifier = "" ) {
		return detail::parse( content, sourceIdentifier );
	}

	inline Node parse( std::istream &stream, const std::string &sourceIdentifier = "" ) {
		std::string content = std::string( std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() );
		return detail::parse( content, sourceIdentifier );
	}

	inline Node parseFile( const std::string &filename ) {
		std::ifstream file( filename, std::ios_base::binary );
		if( file.is_open() ) {
			return parse( file );
		}
		return Node();
	}

	inline std::string emit( const Node &node ) {
		return detail::emit( node );
	}

	inline void emitFile( const std::string &filename, const Node &node ) {
		std::ofstream( filename ) << emit( node );
	}
}