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
#include "gtest.h"

#include "leanTextProcessing.h"

using namespace LeanTextProcessing;

TEST( TextPosition, increment ) {
	TextPosition position, defaultPosition;

	position.increment( false );
	EXPECT_EQ( defaultPosition.column + 1, position.column );
	EXPECT_EQ( defaultPosition.line, position.line );
	EXPECT_EQ( 1, position.index );

	position.increment( true );
	EXPECT_EQ( defaultPosition.column, position.column );
	EXPECT_EQ( defaultPosition.line + 1, position.line );
	EXPECT_EQ( 2, position.index );
}

TEST( TextIterator, isAtEof ) {
	TextContainer emptyContainer( "", "emptyContainer" );

	ASSERT_TRUE( TextIterator( emptyContainer, TextPosition() ).isAtEnd() );
}

TEST( TextIterator, peek ) {
	TextContainer container( "t", "container" );
	TextContainer containerNL( "\n", "containerNL" );
	TextContainer containerNL2( "\r\n", "containerNL2" );
	TextContainer containerNL3( "\r", "containerNL3" );

	EXPECT_EQ( 't', TextIterator( container, TextPosition() ).peek() );
	EXPECT_EQ( '\n', TextIterator( containerNL, TextPosition() ).peek() );
	EXPECT_EQ( '\n', TextIterator( containerNL2, TextPosition() ).peek() );
	EXPECT_EQ( '\n', TextIterator( containerNL3, TextPosition() ).peek() );
}

TEST( TextIterator, next ) {
	{
		TextContainer container( "t", "container" );
		TextIterator iterator( container, TextPosition() );

		iterator.next();
		EXPECT_TRUE( iterator.isAtEnd() );
		EXPECT_EQ( 1, iterator.current.line );
	}

	{
		TextContainer container( "\n", "container" );
		TextIterator iterator( container, TextPosition() );

		iterator.next();
		EXPECT_TRUE( iterator.isAtEnd() );
		EXPECT_EQ( 2, iterator.current.line );
	}

	{
		TextContainer container( "\r\n", "container" );
		TextIterator iterator( container, TextPosition() );

		iterator.next();
		EXPECT_TRUE( iterator.isAtEnd() );
		EXPECT_EQ( 2, iterator.current.line );
	}

	{
		TextContainer container( "\r", "container" );
		TextIterator iterator( container, TextPosition() );

		iterator.next();
		EXPECT_TRUE( iterator.isAtEnd() );
		EXPECT_EQ( 2, iterator.current.line );
	}
}

TEST( TextIterator_Scope, getScopedText ) {
	TextContainer container( "test", "container" );
	TextIterator iterator( container, TextPosition() );

	TextIterator::Scope scope( iterator );
	while( !iterator.isAtEnd() ) {
		iterator.next();
	}

	EXPECT_EQ( "test", scope.getScopedText() );
}

TEST( TextIterator_Helper, read ) {
	TextContainer container( "a\r\nb", "container" );
	TextIterator iterator( container, TextPosition() );

	ASSERT_EQ( 'a', iterator.read() );
	ASSERT_EQ( '\n', iterator.read() );
	ASSERT_EQ( 'b', iterator.read() );
	ASSERT_TRUE( iterator.isAtEnd() );
}

TEST( TextIterator_Helper, tryMatch ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	ASSERT_FALSE( iterator.tryMatch( 'b' ) );
	ASSERT_TRUE( iterator.tryMatch( 'a' ) );
	ASSERT_TRUE( iterator.isAtEnd() );
	ASSERT_FALSE( iterator.tryMatch( 'a' ) );
}

TEST( TextIterator_Helper, tryMatchAny ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	ASSERT_FALSE( iterator.tryMatchAny( "bcdef" ) );
	ASSERT_TRUE( iterator.tryMatchAny( "bcdefa" ) );
	ASSERT_TRUE( iterator.isAtEnd() );
	ASSERT_FALSE( iterator.tryMatchAny( "bcdefa" ) );
}

TEST( TextIterator_Helper, check ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	EXPECT_TRUE( iterator.check( 'a' ) );
	EXPECT_FALSE( iterator.check( 'b' ) );
	iterator.next();
	EXPECT_FALSE( iterator.check( 'b' ) );
}

TEST( TextIterator_Helper, checkNot ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	EXPECT_TRUE( iterator.checkNot( 'b' ) );
	EXPECT_FALSE( iterator.checkNot( 'a' ) );
	iterator.next();
	EXPECT_FALSE( iterator.check( 'b' ) );
}

TEST( TextIterator_Helper, checkAny ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	EXPECT_FALSE( iterator.checkAny( "bcdef" ) );
	EXPECT_TRUE( iterator.checkAny( "bcdefa" ) );
	iterator.next();
	EXPECT_FALSE( iterator.checkAny( "bcdefa" ) );
}

TEST( TextIterator_Helper, checkNotAny ) {
	TextContainer container( "a", "container" );
	TextIterator iterator( container, TextPosition() );

	EXPECT_FALSE( iterator.checkNotAny( "bcdefa" ) );
	EXPECT_TRUE( iterator.checkNotAny( "bcdef" ) );
	iterator.next();
	EXPECT_FALSE( iterator.checkNotAny( "bcdefa" ) );
}

TEST( TextIterator_Helper, readLine ) {
	{
		TextContainer container( "", "container" );
		TextIterator iterator( container, TextPosition() );

		EXPECT_EQ( "\n", iterator.readLine() );
		EXPECT_TRUE( iterator.isAtEnd() );
	}
	{
		TextContainer container( "a", "container" );
		TextIterator iterator( container, TextPosition() );

		EXPECT_EQ( "a\n", iterator.readLine() );
		EXPECT_TRUE( iterator.isAtEnd() );
	}
	{
		TextContainer container( "a\r\n", "container" );
		TextIterator iterator( container, TextPosition() );

		EXPECT_EQ( "a\n", iterator.readLine() );
		EXPECT_TRUE( iterator.isAtEnd() );
	}
}