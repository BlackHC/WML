#include "wml.h"

#include <iostream>
#include <fstream>

void main( int argc, char ** argv ) {
	std::string source = std::string( std::istreambuf_iterator<char>( std::cin ), std::istreambuf_iterator<char>() );

	wml::Node root;
	try {
		root = wml::parse( source );
		std::cout << wml::emit( root );
	}
	catch( std::exception &e ) {
		std::cerr << e.what() << std::endl;
	}
}