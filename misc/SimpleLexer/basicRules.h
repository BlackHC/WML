#pragma once

#include "ruleHelpers.h"

namespace SimpleLexer {
	namespace Rules {
		struct Whitespace {
			bool tryMatch( TextIterator &iterator ) const {
				if( !iterator.matchAny( " \t" ) )
					return false;

				while( iterator.matchAny( " \t" ) )
					;
				return true;
			}

			bool tryRead( TextIterator &iterator, Token &token ) const {
				if( !iterator.checkAny( " \t" ) )
					return false;

				while( iterator.checkAny( " \t" ) ) {
					token.append( iterator.read() );
				}

				return true;
			}
		};

		struct Character {
			char c;

			Character( const char c ) : c( c ) {};

			bool tryMatch( TextIterator &iterator ) const {
				return iterator.tryMatch( c );
			}

			bool tryRead( TextIterator &iterator, Token &token ) const {
				if( iterator.tryMatch( c ) ) {
					token.append( c );
					return true;
				}
				return false;
			}
		};

		struct Newline : Character {
			Newline() : Character( '\n' ) {}
		};

		struct Block {
			const char *block;

			Character( const char *block ) : block( block ) {};

			bool tryMatch( TextIterator &iterator ) const {
				return iterator.tryMatch( block );
			}

			bool tryRead( TextIterator &iterator, Token &token ) const {
				if( iterator.tryMatch( block ) ) {
					token.append( block );
					return true;
				}
				return false;
			}
		};
	}
}