#pragma once

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace SimpleLexer {
	namespace Rules {
#if 0
		struct RuleInterface {
			// if the match oepration fails, iterator won't have been changed
			bool tryMatch( TextIterator &iterator ) const;
			// if the read operation fails, iterator won't have been changed
			bool tryRead( TextIterator &iterator, Token &token ) const;
		};
#endif
	}

	struct TextPosition {
		int line;
		int column;

		int index;

		void increment( bool newLine ) {
			++position;
			if( newLine ) {
				++line;
				column = 1;
			}
		}
	};

	struct TextProcessor {
		std::string textIdentifier;

		std::string text;
		TextIterator iterator;

		TextProcessor( const std::string &text ) : text( text ), iterator( text, position ) {}

		template<typename Rule>
		bool tryMatch( const Rule &rule ) {
			return tryMatch( iterator, rule );
		}

		template<typename Rule>
		bool check( const Rule &rule ) {
			return check( iterator, rule );
		}

		template<typename Rule>
		int skip( const Rule &rule ) {
			return skip( iterator, rule );
		}

		template<typename Rule>
		Token readUntil( const Rule &rule ) {
			return readUntil( iterator, rule );
		}

		template<typename Rule>
		bool tryRead( const Rule &rule, Token &token ) {
			return tryRead( iterator, rule, token );
		}

		template<typename Rule>
		void match( const Rule &rule ) {
			match( iterator, rule );
		}

		template<typename Rule>
		Token read( const Rule &rule ) {
			return read( iterator, rule );
		}
	};

	struct TextIterator {
		const TextProcessor &textProcessor;
		TextPosition current;

		TextIterator( const TextProcessor &textProcessor, const TextPosition &position ) : textProcessor( textProcessor ), current( position ) {}

		bool atEof() const {
			return current.index >= textProcessor.text.size();
		}

		char peek() const {
			// assert !atEof()
			return textProcessor.text[ current.index ];
		}

		void next() {
			current.increment( peek() == '\n' );
		}

		// helper class
		struct Scope {
			TextIterator *iterator;
			TextPosition saved;

			void accept() {
				if( iterator ) {
					saved = iterator->current;
				}
			}

			void reject() {
				if( iterator ) {
					iterator.current = saved;
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
			if( !isAtEof() && peek() == c ) {
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
			if( isAtEof() ) {
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


		bool checkAny( const char *set ) {
			if( isAtEof() ) {
				return false;
			}

			for( const char *p = set ; *p ; ++p ) {
				if( peek() == *p ) {
					return true;
				}
			}
			return false;
		}
	};

	struct Token {
		TextPosition position;
		std::string text;
	};

	template<typename T>
	T as( const Token &token ) {
		return boost::lexical_cast<T>( token.text );
	}

	struct TextException {
		std::string textIdentifier;
		std::string text;
		TextPosition position;

		std::string error;

		std::string message;

		TextException( const TextIterator &iterator, const std::string &error ) : textIdentifier( iterator.textProcessor.textIdentifier ), text( iterator.textProcessor.text ), position( iterator.position ), error( error ) {
			message = boost::str( boost::format( "%s(%i:%i (%i)): %s" ) % textIdentifier % position.line % position.column % position.index % error );
		}

		virtual const char * what() const {
			return message.c_str();
		}
	};

	template<typename Rule>
	inline bool tryMatch( TextIterator &iterator, const Rule &rule ) {
		return rule.tryMatch( iterator );
	}

	template<typename Rule>
	inline bool check( TextIterator iterator, const Rule &rule ) {
		return rule.tryMatch( scopeIterator );
	}

	template<typename Rule>
	inline int skip( TextIterator &iterator, const Rule &rule ) {
		int i = 0;
		while( match( rule ) )
			i++;
		return i;
	}

	template<typename Rule>
	inline Token readUntil( TextIterator &iterator, const Rule &rule ) {
		Token token;
		token.position = iterator.position;

		while( !check( rule ) ) {
			token.append( stream.peek() );
			stream.next();
		}

		return token;
	}

	template<typename Rule>
	inline bool tryRead( TextIterator &iterator, const Rule &rule, Token &token ) {
		token.position = iterator.position;
		return rule.tryRead( iterator, token );
	}

	template<typename Rule>
	inline void match( TextIterator &iterator, const Rule &rule ) {
		if( !match( rule ) ) {
			throw TextException( iterator, "match failed" );
		}
	}

	template<typename Rule>
	inline Token read( TextIterator &iterator, const Rule &rule ) {
		Token token;
		token.position = iterator.position;

		if( rule.tryRead( iterator, token ) ) {
			return token;
		}
		else {
			throw TextException( iterator, "read failed" );
		}
	}	
}