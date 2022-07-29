// 
// project........: testbench
//
// file...........: src/selftest.cpp
//
// author.........: elratmacfat
//
// description....: testbench testing itself.
//
//                  To differentiate between tester and testee, the following naming convention
//                  is applied:
//
//                                      Tester   Testee
//                                      ------   ------
//                  testbench.....:     tb       x
//                  testcase......:     t        y,z
//                  
#include "elrat/testbench.h"

// Operator overloads that throw exceptions, simulating all kind of faulty implementation,
// for example, attempts to read data that is out of range, like with std::vector::at
struct A {
	int value;
};

std::ostream& operator<<(std::ostream&, const A&);
bool operator==(const A&, const A&); 
bool operator<=(const A&, const A&);
bool operator>=(const A&, const A&);
bool operator<(const A&, const A&);
bool operator>(const A&, const A&);

int main() {
	
	using elrat::testbench;

	testbench tb("Testbench Selftest");

	// Create a testee testbench 'x', do some checks that fail, and
	// then in the real testing enviroment, see if those failed
	// checks have been reported properly.
	//
	{
		auto t = tb.create("'testcase' reporting back to 'testbench' on destruction.");
	 	elrat::testbench x("some testbench");
		{
			// testcase that fails with 4 checks in total, 3 of them failed.
			auto y = x.create("some testcase");
			y.check( true );
			y.check( false );
			y.check( false );
			y.check( false );
			// testcase that passes with 1 check.
			auto z = x.create("another testcase");
			z.check( true );
		}
		// y went out of scope, destructor was supposed to report back to x.
		t.equal( x.testcases(), 2 );
		t.equal( x.failed_testcases(), 1 );
		t.equal( x.failed_checks(), 3 );
	}
	// test case: exception handling of comparison performing member functions.
	{
		auto t = tb.create("operator overloads throwing exceptions");
		testbench x("testee testbench");
		t.does_not_throw( [&x](){
			auto y = x.create("testee testcase");
 			// comparison operator overloads for A do throw in any case.
			A a,b;
			y.equal( a, b ); 	
			y.less_than( a, b );
			y.greater_than( a, b );
			y.less_than_or_equal( a, b );
			y.greater_than_or_equal( a, b );
		});
		t.equal( x.failed_testcases(), 1 );
		t.equal( x.failed_checks(), 5 );

	}
	// test case: equal()
	{
		auto t = tb.create("equal, integral types");

		// pass testing
		t.equal( 4, 4);
		t.equal( 4u, 4u );

		// fail testing
		testbench x("testee testbench");
		{
			auto y = x.create("");
			y.equal( 4, 5 );
			y.equal( 5u, 4u );
		}
		t.equal( x.failed_checks(), 2 );

	}
	// test case: equal /w threshold, integral types
	{
		auto t = tb.create("equal, 3 param, integral types");

		// pass testing
		t.equal(5, 3, 2 );
		t.equal( -1, -3, 2 );
		t.equal( 1, 4, 3 );

		// fail testing
		testbench x("testee testbench");
		{
			auto y = x.create("testee testcase");
			y.equal( 5, 3, 1 );
			y.equal( -1, -3, 1 );
			y.equal( 1, 4, 2 );
		}
		t.equal( x.failed_checks(), 3 );

	}
	// test case: equal /w threshold, floating point types
	{
		auto t = tb.create("equal, 3 param, float/double");
		t.equal( 5.0001f, 5.0002f, 0.0001f);
	}
	// test case: less_than
	{
		auto t = tb.create("less_than");
		t.less_than( 5.0001f, 5.00011f );
	}
	// test case: greater_than
	{
		auto t = tb.create("greater_than");
		t.greater_than(5.0001f, 5.00009f);
	}
	// test case: in_range
	{
		auto t = tb.create("in_range");

		// pass testing
		t.in_range( -2, -2, +2 ); // approaching from "below"
		t.in_range( -10, -10, -5 );
		
		t.in_range( +2, -2, +2 ); // approaching from "above"
		t.in_range( -5, -10, -5 );

		// fail testing
		elrat::testbench x("some testbench");
		{
			auto y = x.create("some testcase");
			y.in_range( -3, -2, +2 );
			y.in_range( -11, -10, -5 );

			y.in_range( +3, -3, +2 );
			y.in_range( -4, -10, -5 );
		}
		t.equal( x.failed_testcases(), 1 );
	}
	// test case: not_in_range
	{
		auto t = tb.create("not_in_range");
		// pass testing
		t.not_in_range( -3, -2, +2 ); // approaching from "below"
		t.not_in_range( 0, 1, 2 );
		
		t.not_in_range( +3, -2, +2 );	// approaching from "above"
		t.not_in_range( 3, 1, 2 );

		// fail testing
		testbench x("testee testbench");
		{
			auto y = x.create("testee testcase");
			y.not_in_range( 2, 2, 4 );
			y.not_in_range( 4, 2, 4 );
			y.not_in_range( 0, -5, +5 );
		}
		t.equal( x.failed_checks(), 3 );
	}
	//
	{
		auto t = tb.create("does_not_throw");
		// pass testing
		t.does_not_throw( [](){
			// do nothing
		});
		
		// fail testing
		testbench x("testee testbench");
		{
			auto y = x.create("testee testcase");
			y.does_not_throw([](){
				throw 42;
			});
		}
		t.equal( x.failed_checks(), 1 );
	}
	//
	{
		auto t = tb.create("throws_any()");
		t.throws_any( [](){ 
			throw std::range_error("range_error"); 
		});
	}
	//
	{
		auto t = tb.create("throws_stdexcept()");
		t.throws_stdexcept( [](){ 
			throw std::overflow_error("overflow_error"); 
		});
	}
	//
	{
		auto t = tb.create("throws");
		t.throws<std::range_error>( [](){ 
			throw std::range_error("Das Bandmaß ist voll"); 
		}); 
	}

	std::cout << tb << '\n';

	return tb.failed_testcases();
}



std::ostream& operator<<(std::ostream& os, const A& a) {
	os << a.value;
	return os;
}

bool operator==(const A&, const A&) {
	throw std::out_of_range("");
	return false;
}

bool operator<=(const A&, const A&) {
	throw std::out_of_range("");
	return false;
}

bool operator>=(const A&, const A&) {
	throw std::out_of_range("");
	return false;
}

bool operator<(const A&, const A&) {
	throw std::out_of_range("");
	return false;
}

bool operator>(const A&, const A&) {
	throw std::out_of_range("");
	return false;
}



