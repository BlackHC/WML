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

#include "wml.h"

using namespace wml;

TEST( Parser, empty )  {
	Node node = parse( "" );

	ASSERT_TRUE( node.nodes.empty() );
}

TEST( Parser, oneKey ) {
	Node root = parse( "key value" );

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( "key", root[0].content );
	ASSERT_EQ( "value", root[0].nodes[0].content );
}

TEST( Parser, weirdChars ) {
	Node root = parse( "\x19key" );
	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( "\x19key", root[0].content );
}

TEST( Parser, oneKey_withUnescapedString ) {
	Node root = parse( "'key \\ \"' 'value \\ \"'" );

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( "key \\ \"", root[0].content );
	ASSERT_EQ( "value \\ \"", root[0].nodes[0].content );
}

TEST( Parser, oneKey_withEscapedString ) {
	Node root = parse( "\"key \\\\ \\\"\" \"value \\\\ \\\"\"" );

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( "key \\ \"", root[0].content );
	ASSERT_EQ( "value \\ \"", root[0].nodes[0].content );
}

TEST( Parser, oneKey_withNullEscapedString ) {
	Node root = parse( std::string( "\"key \0 here\" \"value \0 here\"", 27 ) );

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( std::string( "key \0 here", 10 ), root[0].content );
	ASSERT_EQ( std::string( "value \0 here", 12 ), root[0].nodes[0].content );
}


TEST( Parser, oneKey_multipleValues ) {
	Node root = parse( "key valueA valueB" );

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 2, root[0].nodes.size() );
	ASSERT_EQ( "key", root[0].content );
	ASSERT_EQ( "valueA", root[0][0].content );
	ASSERT_EQ( "valueB", root[0][1].content );
}

TEST( Parser, multipleKey_multipleValues ) {
	Node root = parse(
		"keyA valueA valueB\n"
		"keyB valueA valueB" 
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 2, root.nodes.size() );
	ASSERT_EQ( 2, root[0].nodes.size() );
	ASSERT_EQ( 2, root[1].nodes.size() );
	ASSERT_EQ( "keyA", root[0].content );
	ASSERT_EQ( "valueA", root[0][0].content );
	ASSERT_EQ( "valueB", root[0][1].content );
	ASSERT_EQ( "keyB", root[1].content );
	ASSERT_EQ( "valueA", root[1][0].content );
	ASSERT_EQ( "valueB", root[1][1].content );
}

TEST( Parser, multipleKey_multipleValues_keyApi ) {
	Node root = parse(
		"keyA valueA valueB\n"
		"keyB valueA valueB" 
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 2, root.nodes.size() );
	ASSERT_EQ( 2, root[0].nodes.size() );
	ASSERT_EQ( 2, root[1].nodes.size() );
	ASSERT_EQ( "keyA", root["keyA"].content );
	ASSERT_EQ( "valueA", root["keyA"][0].content );
	ASSERT_EQ( "valueB", root["keyA"][1].content );
	ASSERT_EQ( "keyB", root["keyB"].content );
	ASSERT_EQ( "valueA", root["keyB"][0].content );
	ASSERT_EQ( "valueB", root["keyB"][1].content );
}

TEST( Parser, multipleKey_multipleValues_withEmptyLines ) {
	Node root = parse(
		"\n"
		"keyA valueA valueB\n"
		"\n\n\n"
		"keyB valueA valueB"
		"\n\n\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 2, root.nodes.size() );
	ASSERT_EQ( 2, root[0].nodes.size() );
	ASSERT_EQ( 2, root[1].nodes.size() );
	ASSERT_EQ( "keyA", root[0].content );
	ASSERT_EQ( "valueA", root[0][0].content );
	ASSERT_EQ( "valueB", root[0][1].content );
	ASSERT_EQ( "keyB", root[1].content );
	ASSERT_EQ( "valueA", root[1][0].content );
	ASSERT_EQ( "valueB", root[1][1].content );
}

TEST( Parser, nestedMaps ) {
	Node root = parse(
		"keyA:\n"
		"\tkeyB value\n"
		"\tkeyC value\t\tvalue\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 2, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( 2, root["keyA"]["keyC"].nodes.size() );
}

TEST( Parser, nestedMaps_withEmptyLines ) {
	Node root = parse(
		"keyA:\n"
		"\n"
		"\tkeyB value\n"
		"\t\n"
		"\t\t\t   \n"
		"\tkeyC valueA\t\tvalueB\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 2, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( 2, root["keyA"]["keyC"].nodes.size() );
	ASSERT_TRUE( root["keyA"]["keyB"]["value"].nodes.empty() );
	ASSERT_TRUE( root["keyA"]["keyC"]["valueA"].nodes.empty() );
	ASSERT_TRUE( root["keyA"]["keyC"]["valueB"].nodes.empty() );
}

TEST( Parser, nestedNestedMaps_withEmptyLines ) {
	Node root = parse(
		"keyA:\n"
		"\n"
		"\tkeyB value\n"
		"\t\n"
		"\t\t\t   \n"
		"\tkeyC valueA\t\tvalueB\n"
		"\tkeyD:\n"
		"\t\tkeyE valueC\n"
		"\tkeyF\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 4, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( 2, root["keyA"]["keyC"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyD"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyD"]["keyE"].nodes.size() );
	ASSERT_TRUE( root["keyA"]["keyB"]["value"].nodes.empty() );
	ASSERT_TRUE( root["keyA"]["keyC"]["valueA"].nodes.empty() );
	ASSERT_TRUE( root["keyA"]["keyC"]["valueB"].nodes.empty() );
	ASSERT_TRUE( root["keyA"]["keyF"].nodes.empty() );
}

TEST( Parser, nestedMaps_indentedText ) {
	Node root = parse(
		"keyA:\n"
		"\tkeyB::\n"
		"\t\tsome text\n"
		"\t\tthen some more\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 1, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( "some text\nthen some more", root["keyA"]["keyB"].nodes[0].content );
}

TEST( Parser, nestedMaps_indentedText_withEmptyLines ) {
	Node root = parse(
		"keyA:\n"
		"\tkeyB::\n"
		"\n"
		"\t\tsome text\n"
		"\t\t\t\n"
		"\t\tthen some more\n"
		"\n"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 1, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( "\nsome text\n\nthen some more\n", root["keyA"]["keyB"].nodes[0].content );
}

TEST( Parser, nestedMaps_indentedText__withEmptyLines_embedded ) {
	Node root = parse(
		"keyA:\n"
		"\tkeyB::\n"
		"\n"
		"\t\tsome text\n"
		"\t\t\t\n"
		"\t\tthen some more\n"
		"\n"
		"\tkeyC"
		);

	ASSERT_FALSE( root.nodes.empty() );
	ASSERT_EQ( 1, root.nodes.size() );
	ASSERT_EQ( 2, root["keyA"].nodes.size() );
	ASSERT_EQ( 1, root["keyA"]["keyB"].nodes.size() );
	ASSERT_EQ( "\nsome text\n\nthen some more\n", root["keyA"]["keyB"].nodes[0].content );
	ASSERT_TRUE( root["keyA"]["keyC"].nodes.empty() );
}

TEST( Parser, emptyMap ) {
	Node root = parse(
		"\t\t\n"
		);

	ASSERT_TRUE( root.nodes.empty() );
}

TEST( Parser, map_withWrongIndentation ) {
	ASSERT_THROW( parse(
		"keyA:\n"
		"\t\tkeyB value"
		), LeanTextProcessing::TextException );	

	ASSERT_THROW( parse(
		"keyA:\n"
		"keyB value"
		), LeanTextProcessing::TextException );	
}

TEST( API, keyAccess ) {
	Node root = parse( "keyA value\nkeyB" );

	ASSERT_THROW( root[ "KeyB" ], LeanTextProcessing::TextException );
	ASSERT_THROW( root[ "keyC" ], LeanTextProcessing::TextException );
	ASSERT_NO_THROW( root[ "keyA" ][ "value" ] );
	ASSERT_THROW( root[ "keyA" ][ "valueX" ], LeanTextProcessing::TextException );
	ASSERT_NO_THROW( root[ "keyB" ] );
}

TEST( Parser, specialKeys ) {
	Node root = parse( "keyA:B:C::D:E" );
	ASSERT_NO_THROW( root["keyA:B:C::D:E" ] ); 

	ASSERT_THROW( parse( "keyA value:"), LeanTextProcessing::TextException );
}

void emitTest( const char *source ) {
	Node root = parse( source, "original" );
	std::string emitted = emit( root );
	Node root2 = parse( emitted, "emitted" );
	std::string emitted2 = emit( root2 );

	ASSERT_EQ( emitted, emitted2 );
}

TEST( Emitter, empty ) {
	emitTest( "" );
}

TEST( Emitter, simple ) {
	emitTest( "key a b c d e");
	emitTest( "key:\n"
		"\ta\n"
		"\tb 1 2 3\n"
		"\tc:\n"
		"\t\tx 1 2\n"
		"\t\tx 3 4\n"
		);
	emitTest( 
		"key::\n"
		"\tsome data\n"
		"\tmore data\n"
	);
	emitTest( 
		"key::\n"
		"\tsome data\n"
		"\tmore data\n"
		"\tmore data\n"
		"\tmore data\n"
		"\tmore data\n"
		);
}

TEST( Emitter, weirdValues ) {
	emitTest( "'key asd' \"\t\ta\t\" b\\ c d e");
}

TEST( Emitter, emptyContent ) {
	emitTest( "key ''");
	emitTest( "'' ''");
}