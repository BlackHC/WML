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
#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>
#include <utility>

#include "leanTextProcessing.h"

namespace wml {
	struct Node {
		typedef std::vector< Node > NodeContainer;
		typedef NodeContainer::iterator iterator;
		typedef NodeContainer::const_iterator const_iterator;

		LeanTextProcessing::TextContext context;
		std::string content;
		NodeContainer nodes;

		Node & data() {
			if( nodes.empty() ) {
				error( "expected data at node!" );
			}
			else if( nodes.size() > 1 ) {
				error( "expected data at node, found array/map!" );
			}

			return nodes[0];
		}

		const Node & data() const {
			if( nodes.empty() ) {
				error( "expected data at node!" );
			}
			else if( nodes.size() > 1 ) {
				error( "expected data at node, found array/map!" );
			}

			return nodes[0];
		}

		template< typename T >
		T as() const {
			return boost::lexical_cast<T>( content );
		}

		const std::string & key() const {
			return content;
		}

		template< typename T >
		T & get( const std::string &key ) {
			return (*this)[ key ].as<T>();
		}

		template< typename T >
		T & getOr( const std::string &key, const T &defaultValue ) {
			auto node = find( key );

			if( node ) {
				return node->as<T>();
			}

			return defaultValue;
		}

		template< typename T >
		const T & get( const std::string &key ) const {
			return (*this)[ key ].as<T>();
		}

		template< typename T >
		const T & getOr( const std::string &key, const T &defaultValue ) const {
			auto node = find( key );

			if( node ) {
				return node->as<T>();
			}

			return defaultValue;
		}

		iterator find(  const std::string &key ) {
			for( auto node = nodes.begin() ; node != nodes.end() ; ++node ) {
				if( node->content == key ) {
					return node;
				}
			}
			return nodes.end();
		}

		iterator not_found() {
			return nodes.end();
		}

		const_iterator find(  const std::string &key ) const {
			for( auto node = nodes.begin() ; node != nodes.end() ; ++node ) {
				if( node->content == key ) {
					return node;
				}
			}
			return nodes.end();
		}

		iterator begin() {
			return nodes.begin();
		}

		iterator end() {
			return nodes.end();
		}

		const_iterator cbegin() const {
			return nodes.cbegin();
		}

		const_iterator cend() const {
			return nodes.cend();
		}

		Node & operator[] ( const std::string &key ) {
			for( auto item = nodes.begin() ; item != nodes.end() ; ++item ) {
				if( item->content == key ) {
					return *item;
				}
			}

			error( boost::str( boost::format( "key '%s' not found!" ) % key ) );
		}

		Node & operator[] ( int i ) {
			return nodes[ i ];
		}

		size_t size() const {
			return nodes.size();
		}

		bool empty() const {
			return nodes.empty();
		}

		std::vector< iterator > getAll( const std::string &key ) {
			std::vector< iterator > results;

			for( auto item = nodes.begin() ; item != nodes.end() ; ++item ) {
				if( item->content == key ) {
					results.push_back( item );
				}
			}

			return results;
		}

		Node &push_back( Node &&node ) {
			nodes.push_back( node );
			return nodes.back();
		}

		Node &push_back( const Node &node ) {
			nodes.push_back( node );
			return nodes.back();
		}

		template< typename T>
		Node &push_back( const T &value ) {
			push_back( Node( boost::lexical_cast< std::string >( value ) ) );
			return nodes.back();
		}

		Node &push_back( const std::string &value ) {
			push_back( Node( value ) );
			return nodes.back();
		} 

		Node &push_back( std::string &&value ) {
			push_back( Node( std::move( value ) ) );
			return nodes.back();
		} 

		template< typename T >
		void set( const T &value ) {
			content = boost::lexical_cast< std::string >( value );
		}

		Node() {}
		Node( std::string &&content ) : content( std::move( content ) ) {}
		Node( const std::string &content ) : content( content ) {}
		Node( const std::string &content, const LeanTextProcessing::TextContext &context ) : content( content ), context( context ) {}

		Node( Node &&node ) : content( std::move( node.content ) ), nodes( std::move( node.nodes ) ), context( std::move( node.context ) ) {}

		Node & operator == ( Node &&node ) {
			content = std::move( node.content );
			nodes = std::move( node.nodes );
			context = std::move( node.context );
		}

		void error( const std::string &message ) const {
			throw LeanTextProcessing::TextException( context, message );
		}
	};
}