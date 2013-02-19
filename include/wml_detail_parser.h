// for parseFile and parse (stream overload)
#include <istream>

namespace wml {
	namespace detail {
		using namespace LeanTextProcessing;

		struct Parser {
			int indentLevel;
			TextIterator textIterator;

			std::string parseIdentifier() {
				std::string text;
				while( textIterator.checkNotAny( " \t\n" ) ) {
					if( textIterator.check( ':' ) ) {
						TextIterator::Scope scope( textIterator );
						textIterator.tryMatch( "::" ) || textIterator.tryMatch( ':' );

						if( textIterator.checkNotAny( " \t\n" ) ) {
							text.append( scope.getScopedText() );
							scope.accept();
						}
						else {
							scope.reject();
							break;
						}
					}
					text.push_back( textIterator.read() );
				}

				if( text.empty() ) {
					textIterator.error( "expected identifier!" );
				}

				return text;
			}

			void skipWhitespace() {
				while( textIterator.tryMatchAny( " \t" ) )
					;
			}

			void skipEmptyLines() {
				TextIterator::Scope scope( textIterator );

				while( true ) {
					skipWhitespace();

					if( !textIterator.tryMatch( '\n' ) ) {
						break;
					}

					scope.accept();
				}
			}

			std::string parseEscapedString() {
				std::string text;

				if( !textIterator.tryMatch( '"' ) ) {
					textIterator.error( "'\"' expected!" );
				}
				while( textIterator.checkNotAny( "\"\n" ) ) {
					if( textIterator.tryMatch( '\\' ) ) {
						if( textIterator.atEof() ) {
							textIterator.error( "unexpected EOF!" );
						}
						const char control = textIterator.read();
						switch( control ) {
						case '\\':
						case '\'':
						case '\"':
							text.push_back( control );
							break;
						case 't':
							text.push_back( '\t' );
							break;
						case 'n':
							text.push_back( '\n' );
							break;
						case 'r':
							text.push_back( '\r' );
							break;
						case '0':
							text.push_back( '\0' );
							break;
						default:
							textIterator.error( boost::str( boost::format( "unknown escape control character '%c'!" ) % control  ) );
						}
					}
					else {
						text.push_back( textIterator.read() );
					}
				}
				if( !textIterator.tryMatch( '"' ) ) {
					textIterator.error( "'\"' expected!" );
				}

				return text;
			}

			std::string parseUnescapedString() {
				std::string text;

				if( !textIterator.tryMatch( '\'' ) ) {
					textIterator.error( "' expected!" );
				}
				while( textIterator.checkNotAny( "'\n" ) ) {
					text.push_back( textIterator.read() );
				}
				if( !textIterator.tryMatch( '\'' ) ) {
					textIterator.error( "' expected!" );
				}

				return text;
			}

			void skipIndentLevel() {
				for( int i = 0 ; i < indentLevel ; ++i ) {
					if( !textIterator.tryMatch( '\t' ) ) {
						textIterator.error( boost::str( boost::format( "expected %i tabs - found only %i!") % indentLevel % i  ) );
					}
				}
			}

			bool checkMinimumIndentLevel() {
				TextIterator::Scope scope( textIterator);

				for( int i = 0 ; i < indentLevel ; ++i ) {
					if( !textIterator.tryMatch( '\t') ) {
						return false;
					}
				}
				return true;
			}

			bool checkIndentLevel() {
				TextIterator::Scope scope( textIterator );

				for( int i = 0 ; i < indentLevel ; ++i ) {
					if( !textIterator.tryMatch( '\t') ) {
						return false;
					}
				}

				if( textIterator.check( '\t' ) ) {
					return false;
				}

				return true;
			}

			void ensureIndentLevel() {
				skipIndentLevel();

				if( textIterator.check( '\t' ) ) {
					textIterator.error( boost::str( boost::format( "expected %i tabs - found more!") % indentLevel ) );
				}
			}

			bool checkRestOfLineEmpty() {
				TextIterator::Scope scope( textIterator );				

				while( textIterator.tryMatchAny( " \t" ) )
					;

				return textIterator.peek() == '\n';
			}

			std::string parseIndentedText() {
				std::string text;

				bool isEmptyLine;
				while( !textIterator.atEof() && ( (isEmptyLine = checkRestOfLineEmpty()) || checkMinimumIndentLevel() )  ) {
					if( isEmptyLine ) {
						textIterator.readLine();
						text.push_back( '\n' );
					}
					else {
						skipIndentLevel();

						text.append( textIterator.readLine() );	
					}
				}

				// remove the last newline
				if( !text.empty() ) {
					text.pop_back();
				}

				return text;
			}

			std::string parseValue() {
				if( textIterator.check( '"' ) ) {
					return parseEscapedString();
				}
				else if( textIterator.check( '\'' ) ) {
					return parseUnescapedString();
				}
				else {
					return parseIdentifier();
				}
			}

			void expectNewline() {
				if( !textIterator.tryMatch( '\n' ) ) {
					textIterator.error( "expected newline!" );
				}
			}

			void parseMap( Node &node, bool allowEmpty = true ) {
				while( true ) {
					skipEmptyLines();

					if( !checkMinimumIndentLevel() || textIterator.atEof() ) {
						break;
					}

					ensureIndentLevel();
					skipWhitespace();

					std::string key = parseValue();

					Node childNode( key, TextContext( textIterator ) );

					skipWhitespace();

					if( textIterator.tryMatch( ':' ) ) {
						skipWhitespace();

						if( textIterator.tryMatch( ':' ) ) {
							// text literal
							skipWhitespace();
							expectNewline();

							indentLevel++;
							std::string indentedText = parseIndentedText();
							indentLevel--;

							childNode.nodes.push_back( Node( indentedText, TextContext( textIterator ) ) );
						}
						else {
							expectNewline();

							indentLevel++;
							parseMap( childNode, false );
							indentLevel--;
						}
					}
					else {
						parseInlineValues( childNode );
					}

					node.nodes.push_back( std::move( childNode ) );
				}
				if( node.nodes.empty() && !allowEmpty ) {
					textIterator.error( "expected non-empty map" );
				}
			}

			void parseInlineValues( Node &node ) {
				while( !textIterator.atEof() && !textIterator.tryMatch( '\n' ) ) {
					std::string value = parseValue();

					node.nodes.push_back( Node( value, TextContext( textIterator ) ) );

					skipWhitespace();
				}
			}

			Parser( const TextContainer &textContainer ) : indentLevel( 0 ), textIterator( textContainer, TextPosition() ) {}
		};

		inline Node parse( const std::string &content, const std::string &sourceIdentifier ) {
			TextContainer textContainer( content, sourceIdentifier );

			Parser parser( textContainer );

			Node node( sourceIdentifier );
			parser.parseMap( node );
			return node;
		}
	}
}