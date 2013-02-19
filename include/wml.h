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