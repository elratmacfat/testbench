//
// project........: testbench
//
// file...........: src/example.cpp 
//
// author.........: elratmacfat
//
// description....: shows how the testbench-library can be used.
//
#include <functional>
#include <iostream>
#include "elrat/testbench.h"

int main( int argc, char* argv[] )
{

	elrat::testbench tb("Example Testbench");

	// First testcase to demonstrate the usage of scope blocks
	{ 
		elrat::testbench::testcase t = tb.create("First testcase");

		t.check( true );

		// testcase 't' will now go out of scope and upon destruction 
		// it reports its results back to its parent testbench 'tb'.
	}

	// Simple true/false checking
	{
		auto t = tb.create("Use of testing function");

		// 
		t.check( true );	// OK
		t.check( false );	// FAILS. When printing the results, 
					// this one is marked #2, because it's
					// the second check call within this 
					// context.
	}

	// Comparing two values/objects
	{
		auto t = tb.create("Comparison");
		t.equal( 1, 2 );		// FAILS (#1)
		t.equal( 1, 1 );		// OK
		t.less_than( 1, 2 );		// OK, 1 < 2
		t.greater_than( 5.15f, 3.31f );	// OK, 5.15 > 3.31
		t.less_than_or_equal( 2, 2 );	// OK
		t.greater_than_or_equal( 2, 2);	// OK
	}

	// Floating Point Comparison
	{
		auto t = tb.create("Floating point comparison");
		t.equal( 1.0008f, 1.0009f );  		// FAILS

		// Add a third parameter, that tells how close the values have 
		// to be to consider them equal.
		t.equal( 1.0008f, 1.0009f, 0.0005f ); 	// OK
	}
	
	std::cout << tb;
	return 0;
}
