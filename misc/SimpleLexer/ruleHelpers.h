#pragma once

#include "textProcessor.h"

namespace SimpleLexer {
	namespace Rules {
		template< class SimpleRule >
		struct WrapScope {
			// implement:
			//	 bool simpleTryMatch( TextIterator &iterator ) const;
			//	 bool simpleTryRead( TextIterator &iterator, Token &token ) const;
			// read can return an iterator in any state if it fails

			bool doSimpleTryMatch( TextIterator &iterator ) const {
				return reinterpret_cast<SimpleRule*>(*this)->simpleTryMatch( iterator );
			}

			bool doSimpleTryRead( TextIterator &iterator, Token &token ) const {
				return reinterpret_cast<SimpleRule*>(*this)->simpleTryRead( iterator, token );
			}

			// if the read operation fails, iterator won't have been changed
			bool tryRead( TextIterator &iterator, Token &token ) const {
				TextIterator::Scope scope( iterator );

				if( doSimpleTryRead( iterator, token ) ) {
					scope.release();
					return true;
				}
				return false;
			}

			// if the match oepration fails, iterator won't have been changed
			bool tryMatch( TextIterator &iterator ) const {
				TextIterator::Scope scope( iterator );

				if( doSimpleTryMatch( iterator, token ) ) {
					scope.release();
					return true;
				}
				return false;
			}
		};

		template< class SimpleRule >
		struct CompleteSimpleRule : WrapScope< CompleteSimpleRule< SimpleRule > > {
			// only implement bool simpleTryRead( TextIterator &iterator, Token &token ) const;
			// read can return an iterator in any state if it fails

			bool simpleTryMatch( const TextIterator &iterator ) const {
				Token token;
				return simpleTryRead( iterator, token );
			}
		};

		template< class RuleA, class RuleB >
		struct RuleOr {
			RuleA a;
			RuleB b;

			RuleOr( const RuleA &a, const RuleB &b ) : a( a ), b( b ) {}

			bool tryMatch( TextIterator &iterator ) const {
				return a.tryMatch( iterator ) || b.tryMatch( iterator );
			}

			bool tryRead( TextIterator &iterator, Token &token ) const {
				return a.tryRead( iterator, token ) || b.tryRead( iterator, token );
			}
		};

		template< class RuleA, class RuleB >
		inline RuleOr< RuleA, RuleB > operator | ( const RuleA &a, const RuleB &b ) {
			return RuleOr( a, b );
		}

		template< class RuleA, class RuleB >
		struct RuleConcat : ScopedRuleBase< RuleConcat< RuleA, Ruleb > >  {
			RuleA a;
			RuleB b;

			RuleConcat( const RuleA &a, const RuleB &b ) : a( a ), b( b ) {}

			bool simpleTryMatch( TextIterator &iterator ) const {
				if( !a.tryMatch( iterator ) ) {
					return false;
				}
				if( b.tryMatch( iterator ) ) {
					return true;
				}
				return false;
			}

			bool simpleTryRead( TextIterator &iterator, Token &token ) const {
				Token secondToken;
				bool matched = a.tryRead( iterator, token ) && b.tryRead( iterator, secondToken );
				token.text.append( secondToken.text );
				return matched;
			}
		};

		template< class RuleA, class RuleB >
		inline RuleConcat< RuleA, RuleB > operator +( const RuleA &a, const RuleB &b ) {
			return RuleConcat( a, b );
		}

	}
}