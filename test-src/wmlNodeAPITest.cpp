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

TEST( API, empty_size_index_key_value ) {
	const char *text = "keyA valueA\nkeyB valueB";

	Node root;

	ASSERT_TRUE( root.empty() );

	root = parse( text );

	ASSERT_FALSE( root.empty() );

	int numRootNodes = root.size();
	ASSERT_EQ( 2, numRootNodes );

	auto node0Key = root[0].key();
	auto node0Value = root[0].value();
	ASSERT_EQ( "keyA", node0Key );
	ASSERT_EQ( "valueA", node0Value );

	auto node1Key = root[1].key();
	auto node1Value = root[1].value();
	ASSERT_EQ( "keyB", node1Key );
	ASSERT_EQ( "valueB", node1Value );

	// we can also set key and value
	root[0].key() = "keyAA";
	root[0].value() = "valueAA";
	root[1].key() = "keyBB";
	root[1].value() = "valueBB";
}

TEST( API, iterators ) {
	const char *text = "keyA valueA\nkeyB valueB";

	Node root = parse( text );

	{
		Node::iterator it = root.begin();
		ASSERT_EQ( "keyA", it->key() );
		it->key() = "keyAA";
		++it;
		ASSERT_EQ( "keyB", it->key() );
		it->key() = "keyBB";
		++it;
		ASSERT_EQ( root.end(), it );
	}

	{
		Node::const_iterator it = root.cbegin();
		ASSERT_EQ( "keyAA", it->key() );
		++it;
		ASSERT_EQ( "keyBB", it->key() );
		++it;
		ASSERT_EQ( root.end(), it );
	}
}

TEST( API, find ) {
	const char *text = "keyA valueA\nkeyB valueB";

	Node root = parse( text );

	ASSERT_EQ( root.begin(), root.find( "keyA" ) );
	ASSERT_EQ( root.end(), root.find( "keyX" ) );
}

TEST( API, index_by_key ) {
	const char *text = "keyA valueA\nkeyB valueB";

	Node root = parse( text );

	auto nodeA = root["keyA"];
	ASSERT_EQ( "keyA", nodeA.key() );
	ASSERT_EQ( "valueA", nodeA.value() );

	auto nodeB = root["keyB"];
	ASSERT_EQ( "keyB", nodeB.key() );
	ASSERT_EQ( "valueB", nodeB.value() );
}

TEST( API, get_as ) {
	const char *text = "pie 413\npi 3.1415";

	Node root = parse( text );
	
	int pieValue = root[ "pie" ].as<int>();
	ASSERT_EQ( 413, pieValue );

	float piValue = root.get<float>( "pi" );
	ASSERT_FLOAT_EQ( 3.1415, piValue );
}

TEST( API, setValue ) {
	Node node;

	node.setValue( 31415 );

	ASSERT_EQ( 31415, node.as<int>() );

	node.setValue( 3.1415 );
	ASSERT_FLOAT_EQ( 3.1415, node.as<float>() );
}

TEST( API, getOr ) {
	Node root;

	ASSERT_EQ( 1, root.getOr<int>( "key", 1 ) );

	root = parse( "key 0" );
	ASSERT_EQ( 0, root.getOr<int>( "key", 1 ) );
}

TEST( API, getNodes ) {
	Node root = parse( "value 0\nvalue 1\nvalue 2\notherKey" );

	ASSERT_TRUE( root.getNodes( "key" ).empty() );

	auto values = root.getNodes( "value" );
	ASSERT_EQ( 3, values.size() );

	for( int i = 0 ; i < 3 ; i++ ) {
		ASSERT_EQ( i, values[i]->as<int>() );
	}
}

TEST( API, push_back ) {
	Node root;

	root.push_back( "key" ).push_back( "value" );
	root.push_back( "pie" ).push_back( 413 );

	ASSERT_EQ( 2, root.size() );
	ASSERT_EQ( "value", root[ "key" ].value() );
	ASSERT_EQ( 413, root.get<int>( "pie" ) );
}