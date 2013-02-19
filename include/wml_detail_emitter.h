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

				// null characters => VT_ESCAPED_STRING
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

			void emitMap( const Node &node ) {
				for( auto item = node.nodes.begin() ; item != node.nodes.end() ; ++item ) {
					emitTabs();
					// emit the key
					emitValue( item->content, true );

					if( isMap( *item ) ) {
						text.append( ":\n" );
						++indentLevel;
						emitMap( *item );
						--indentLevel;
					}
					else if( isTextBlock( *item ) ) {
						emitValue( item->data().content, false );
					}
					else {
						emitInlineValues( *item );
					}
				}
			}
		};

		inline std::string emit( const Node &node ) {
			Emitter emitter;
			emitter.emitMap( node );
			return emitter.text;
		}
	}
}