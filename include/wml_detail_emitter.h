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
// for emitFile
#include <fstream>

namespace wml {
	namespace detail {
		struct Emitter {
			int indentLevel;
			std::string text;

			Emitter() : indentLevel( 0 ) {}

			void emitTabs() {
				for( int i = 0 ; i < indentLevel ; ++i ) {
					text.push_back( '\t' );
				}
			}

			enum ValueType {
				VT_IDENTIFIER,
				VT_UNESCAPED_STRING,
				VT_ESCAPED_STRING,
				VT_TEXT
			};

			static ValueType determineType( const std::string &value ) {
				int numLines = 1;

				if( value.empty() ) {
					return VT_UNESCAPED_STRING;
				}

				// null characters or special characters => VT_ESCAPED_STRING
				for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
					if( *c == 0 || *c == '\r' ) {
						return VT_ESCAPED_STRING;
					}
				}

				for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
					if( *c == '\n' ) {
						numLines++;
					}
				}

				// raw text only for more than 2 lines
				if( numLines > 2 ) {
					return VT_TEXT;
				}

				for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
					if( *c == '\n' || *c == '\t' || *c == '\'' ) {
						return VT_ESCAPED_STRING;
					}
				}

				for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
					if( *c == ' ' || *c == ':' ) {
						return VT_UNESCAPED_STRING;
					} 
				}

				return VT_IDENTIFIER;
			}

			void emitValue( const std::string &value, bool noTextBlocks ) {
				ValueType vt = determineType( value );

				if( vt == VT_TEXT && noTextBlocks ) {
					vt = VT_ESCAPED_STRING;
				}

				if( vt == VT_IDENTIFIER ) {
					text.append( value );
				}
				else if( vt == VT_UNESCAPED_STRING ) {
					text.push_back( '\'' );
					text.append( value );
					text.push_back( '\'' );
				}
				else if( vt == VT_TEXT ) {
					text.append( "::\n" );

					++indentLevel;

					emitTabs();
					for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
						text.push_back( *c );
						if( *c == '\n' ) {
							emitTabs();
						}
					}				

					text.push_back( '\n' );
					--indentLevel;
				}
				else if( vt == VT_ESCAPED_STRING ) {
					text.push_back( '\"' );

					for( auto c = value.cbegin() ; c != value.cend() ; ++c ) {
						if( *c == '\\' || *c == '\"' ) {
							text.push_back( '\\' );
							text.push_back( *c );
						}
						else if( *c == '\t' ) {
							text.append( "\\t" );
						}
						else if( *c == '\n' ) {
							text.append( "\\n" );
						}
						else if( *c == '\r' ) {
							text.append( "\\r" );
						}
						else if( *c == '\0' ) {
							text.append( "\\0" );
						}
						else {
							text.push_back( *c );
						}
					}

					text.push_back( '\"' );
				}
			}

			static bool isMap( const Node &node ) {
				for( auto item = node.nodes.begin() ; item != node.nodes.end() ; ++item ) {
					if( !item->empty() ) {
						return true;
					}
				}
				return false;
			}

			static bool isTextBlock( const Node &node ) {
				return node.size() == 1 && determineType( node.nodes[0].content ) == VT_TEXT;
			}

			void emitInlineValues( const Node &node ) {
				for( auto item = node.nodes.begin() ; item != node.nodes.end() ; ++item ) {
					text.push_back( ' ' );

					emitValue( item->content, true );
				}
				text.push_back( '\n' );
			}

			void emitNode( const Node &node ) {
				for( auto item = node.nodes.begin() ; item != node.nodes.end() ; ++item ) {
					emitTabs();
					// emit the key
					emitValue( item->content, true );

					if( isMap( *item ) ) {
						text.append( ":\n" );
						++indentLevel;
						emitNode( *item );
						--indentLevel;
					}
					else if( isTextBlock( *item ) ) {
						emitValue( item->nodes[0].content, false );
					}
					else {
						emitInlineValues( *item );
					}
				}
			}
		};

		inline std::string emit( const Node &node ) {
			Emitter emitter;
			emitter.emitNode( node );
			return emitter.text;
		}
	}
}