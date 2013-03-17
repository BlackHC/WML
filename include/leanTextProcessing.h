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

#include <boost/format.hpp>
#include <string>
#include <exception>
#include <assert.h>

namespace LeanTextProcessing {
	struct TextPosition {
		int line;
		int column;

		int index;

		TextPosition() : line( 1 ), column( 1 ), index( 0 ) {}

		void increment( bool newLine ) {
			++index;
			if( newLine ) {
				++line;
				column = 1;
			}
			else {
				++column;
			}
		}
	};

	// named text
	struct TextContainer {
		std::string name;
		std::string text;

		// TODO: change parameter order [3/17/2013 Andreas]
		TextContainer( const std::string &text, const std::string &name ) 
			: text( text )
			, name( name )
		{}
	};

	struct TextException;

	// reads in text from a TextContainer 
	struct TextIterator {
		const TextContainer &textContainer;
		TextPosition current;

		TextIterator( const TextContainer &textContainer, TextPosition position )
			: textContainer( textContainer )
			, current( position ) 
		{}

		bool isAtEnd() const {
			return current.index >= (int) textContainer.text.size();
		}

		// always returns \n for newlines
		char peek() const {
			assert( !isAtEnd() );

			const char c = textContainer.text[ current.index ];
			if( c == '\r' ) {
				return '\n';
			}
			return c;
		}

		// supports *NIX, Windows and MacOs newlines
		void next() {
			assert( !isAtEnd() );

			bool newLine = false;
			if( textContainer.text[ current.index ] == '\r' ) {
				newLine = true;

				if( textContainer.text[ current.index + 1 ] == '\n' ) {
					++current.index;
				}
			}
			else if( textContainer.text[ current.index ] == '\n' ) {
				newLine = true;
			}
			current.increment( newLine );
		}

		// helper class
		// read in text provisionally and restore original position on leaving the scope (by default)
		struct Scope {
			TextIterator &iterator;
			TextPosition saved;

			// note: newlines won't have been converted
			std::string getScopedText() {
				return iterator.textContainer.text.substr( saved.index, iterator.current.index - saved.index );
			}

			// accept the parsed text
			// (useful for partial accepts)
			void accept() {
				saved = iterator.current;
			}

			// reject the text and rewind to the stored position
			void reject() {
				iterator.current = saved;
			}

			Scope( TextIterator &iterator ) : iterator( iterator ), saved( iterator.current ) {}
			~Scope() {
				reject();
			}
		};

		// helper methods
		char read() {
			char c = peek();
			next();
			return c;			
		}
		
		bool tryMatch( const char c ) {
			if( !isAtEnd() && peek() == c ) {
				next();
				return true;
			}
			return false;
		}

		bool tryMatch( const char *text ) {
			Scope scope( *this );

			for( const char *p = text ; *p ; ++p ) {
				if( isAtEnd() || peek() != *p ) {
					return false;
				}
				next();
			}

			scope.accept();
			return true;
		}

		bool tryMatchAny( const char *set ) {
			if( isAtEnd() ) {
				return false;
			}

			const char c = peek();
			for( const char *p = set ; *p ; ++p ) {
				if( c == *p ) {
					next();
					return true;
				}
			}
			return false;
		}

		bool check( const char c ) const {
			return !isAtEnd() && peek() == c;
		}

		bool checkNot( const char c ) const {
			return !isAtEnd() && peek() != c;
		}

		bool checkAny( const char *set ) {
			if( isAtEnd() ) {
				return false;
			}

			const char c = peek();
			for( const char *p = set ; *p ; ++p ) {
				if( c == *p ) {
					return true;
				}
			}
			return false;
		}

		bool checkNotAny( const char *cset ) {
			if( isAtEnd() ) {
				return false;
			}

			const char c = peek();
			for( const char *p = cset ; *p ; ++p ) {
				if( c == *p ) {
					return false;
				}
			}
			return true;
		}

		// read a full line
		// always contains a new line at the end
		std::string readLine() {
			std::string text;

			while( checkNot( '\n' ) ) {
				text.push_back( read() ); 
			}

			tryMatch( '\n' );
			text.push_back( '\n' );

			return text;
		}

		void error( const std::string &error );
	};

	// instead of keeping the whole container, a context stores relevant information for error processing
	struct TextContext {
		std::string name;
		TextPosition position;
		std::string surroundingText;

		TextContext() {}

		TextContext( TextContext &&context ) 
			: name( std::move( context.name ) )
			, position( std::move( context.position ) )
			, surroundingText( std::move( context.surroundingText ) ) 
		{}

		TextContext( const TextIterator &iterator, int contextWidth = 30 ) 
			: name( iterator.textContainer.name )
			, position( iterator.current )
			, surroundingText( 
				iterator.textContainer.text.substr( std::max( 0, position.index - contextWidth ), contextWidth ) 
					+ "*HERE*" + iterator.textContainer.text.substr( std::max( 0, position.index ), contextWidth )
				) 
		{}
	};

	struct TextException : std::exception {
		TextContext context;
		std::string error;

		std::string message;

		TextException( const TextContext &context, const std::string &error ) 
			: context( context )
			, error( error )
		{
			message = boost::str( 
				boost::format( "%s(%i:%i (%i)): %s\n\t%s\n" ) 
					% context.name 
					% context.position.line 
					% context.position.column 
					% context.position.index 
					% error 
					% context.surroundingText
				);
		}

		virtual const char * what() const {
			return message.c_str();
		}
	};

	inline void TextIterator::error( const std::string &error ) {
		throw TextException( TextContext( *this ), error );
	}
}
