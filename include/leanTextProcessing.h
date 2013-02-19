#pragma once

#include <boost/format.hpp>
#include <string>
#include <exception>

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
		}
	};

	struct TextContainer {
		std::string textIdentifier;

		std::string text;

		TextContainer( const std::string &text, const std::string &textIdentifier ) : text( text ), textIdentifier( textIdentifier ) {}
	};

	struct TextException;

	struct TextIterator {
		const TextContainer &textContainer;
		TextPosition current;

		TextIterator( const TextContainer &textContainer, const TextPosition &position ) : textContainer( textContainer ), current( position ) {}

		bool atEof() const {
			return current.index >= (int) textContainer.text.size();
		}

		char peek() const {
			// assert !atEof()
			const char c = textContainer.text[ current.index ];
			if( c == '\r' ) {
				return '\n';
			}
			return c;
		}

		void next() {
			bool newLine = false;
			if( textContainer.text[ current.index ] == '\r' ) {
				newLine = true;

				if( textContainer.text[ current.index + 1 ] == '\n' ) {
					++current.index;
				}
			}
			current.increment( newLine );
		}

		// helper class
		struct Scope {
			TextIterator *iterator;
			TextPosition saved;

			std::string getScopedText() {
				return iterator->textContainer.text.substr( saved.index, iterator->current.index - saved.index );
			}

			void accept() {
				if( iterator ) {
					saved = iterator->current;
				}
			}

			void reject() {
				if( iterator ) {
					iterator->current = saved;
				}
			}

			void release() {
				iterator = nullptr;
			}

			Scope( TextIterator &iterator ) : iterator( &iterator ), saved( iterator.current ) {}
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
			if( !atEof() && peek() == c ) {
				next();
				return true;
			}
			return false;
		}

		bool tryMatch( const char *text ) {
			for( const char *p = text ; *p ; ++p ) {
				if( atEof() || peek() != *p ) {
					return false;
				}
				next();
			}
			return true;
		}

		bool tryMatchAny( const char *set ) {
			if( atEof() ) {
				return false;
			}

			for( const char *p = set ; *p ; ++p ) {
				if( peek() == *p ) {
					next();
					return true;
				}
			}
			return false;
		}

		bool check( const char c ) const {
			return !atEof() && peek() == c;
		}

		bool checkNot( const char c ) const {
			return !atEof() && peek() != c;
		}

		bool checkAny( const char *set ) {
			if( atEof() ) {
				return false;
			}

			for( const char *p = set ; *p ; ++p ) {
				if( peek() == *p ) {
					return true;
				}
			}
			return false;
		}

		bool checkNotAny( const char *cset ) {
			if( atEof() ) {
				return false;
			}

			for( const char *p = cset ; *p ; ++p ) {
				if( peek() == *p ) {
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

	struct TextContext {
		std::string textIdentifier;
		TextPosition position;
		std::string surroundingText;

		TextContext() {}

		TextContext( TextContext &&context ) 
			: 
				textIdentifier( std::move( context.textIdentifier ) ), 
				position( std::move( context.position ) ),
				surroundingText( std::move( context.surroundingText ) ) 
		{}

		TextContext( const TextIterator &iterator, int contextWidth = 30 ) 
			: textIdentifier( iterator.textContainer.textIdentifier ), 
				position( iterator.current ),
				surroundingText( iterator.textContainer.text.substr( std::max( 0, position.index - contextWidth ), contextWidth ) + "*HERE*" + iterator.textContainer.text.substr( std::max( 0, position.index ), contextWidth ))
			{}
	};

	struct TextException : std::exception {
		TextContext context;
		std::string error;

		std::string message;

		TextException( const TextContext &context, const std::string &error ) : context( context ), error( error ) {
			message = boost::str( 
				boost::format( "%s(%i:%i (%i)): %s\n\t%s\n" ) 
					% context.textIdentifier 
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
