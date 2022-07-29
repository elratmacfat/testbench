// 
// project........: testbench
//
// file...........: elrat/testbench.h
//
// author.........: elratmacfat
//
// description....: tiny testing library
// 
//                  compiler support for C++11 is required
//                
//                  on how to use this testing library, please refer to src/example.cpp
//                  or src/selftest.cpp
//
#ifndef ELRAT_TESTBENCH_H
#define ELRAT_TESTBENCH_H

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <type_traits>
#include <vector>

namespace elrat {

//------------------------------------- DECLARATION -----------------------------------------------

class testbench {
private:
	struct log;
	struct intern;

	std::string m_name;
	std::vector<log> m_logs;

	// called by child testcases to submit their results
	void add( log&& log );
public:
	class testcase;
	friend class testcase;

	// Constructor takes a descriptive, but otherwise irrelevant name.
	testbench( const std::string& name );

	// Creates a testcase (which has the member functions that do the testing)
	testcase create( const std::string& name );

	// 'getter'
	const std::string& name() const;
	int testcases() const;
	int checks() const;
	int failed_testcases() const;
	int failed_checks() const;
	const std::vector<log>& logs() const;
};

struct testbench::log {
	struct entry;
	log( const std::string& name );
	log( const log& ) = delete;
	log( log&& ) = default;
	void add( bool failed, const std::string& msg );
	std::string name;
	int check_count;
	std::vector<entry> entries;
};

struct testbench::log::entry {
	entry( int pos, std::string msg="" );
	entry( const entry& ) = delete;
	entry( entry&& ) = default;
	int position;
	std::string message;
};

class testbench::testcase {
private:
	
	// testbench is a friend, because it has to call the private constructor.
	friend class testbench;

	// private data member
	log m_log;
	testbench* m_parent;

	// private function member 
	
	// constructor, invoked by the parent testbench.
	testcase( const std::string& name, testbench* parent );

	// This private function is used to encapsulate comparison operations in a try-catch-block.
	// While this is not necessary for primitive data types, the application programmer might 
	// have own overloads, which are not necessarily guaranteed to nothrow.
	template <class Callable>
	void encapsulate_comparison( std::string&& s, Callable&& c );
public:
	// Deleted copy constructor, because on when a testcase goes out of scope, it'll report
	// back to the parent testbench, which shall occur only once...
	testcase( const testcase& ) = delete; 

	// ... Instead the object will be moved around if necessary.
	testcase( testcase&& ) = default;

	// When destroyed the object will add its results to the parent testbench.
	~testcase();

	// Requires T to be implicitely convertible to bool
	template <class T> void check( const T& );
	
	// The following members require T to have an overloaded output operator<< 
	// because if the check fails, the arguments will be written into a stringstream 
	// to create a human-readable feedback message
	
	// Requires overloaded operator==
	template <class T> void equal( const T& a, const T& b );
	
	// Requires overloaded operator< 
	// Checks if a is in range [b-th,b+th]
	template <class T> void equal( const T& a, const T& b, const T& th ); 

	// Requires overloaded operator< 
	template <class T> void less_than( const T& a, const T& b );
	
	// Requires overloaded operator<=
	template <class T> void less_than_or_equal( const T& a, const T& b );
	
	// Requires overloaded operator>
	template <class T> void greater_than( const T& a, const T& b );
	
	// Requires overloaded operator>=
	template <class T> void greater_than_or_equal( const T& a, const T& b );

	// Requires operator< and operator> 
	template <class T> void in_range( const T& a, const T& lo, const T& hi );

	// Requires operator< and operator> 
	template <class T> void not_in_range( const T& a, const T& lo, const T& hi );

	// Type 'Callable' is callable when operator() without any arguments can invoked
	// on the object, e.g. lambda, std::bind, function pointer or std::function
	
	// Must not throw
	template <class Callable> void does_not_throw( Callable&& callable );
	// Require anything to be caught.
	template <class Callable> void throws_any( Callable&& callable );
	// Require any exception that implements the std::exception interface
	template <class Callable> void throws_stdexcept( Callable&& callable );
	// Require a specific type of exception (even non-standard types)
	template <class Exception, class Callable> void throws( Callable&& callable );

}; 

// Utility function to create detailed error/failed message.
// Example: 
// 	std::string msg = intern::concatenate(
// 		"Err #", 42, ". Answer given is [", 5.15, "], but should be over ", 9000, "!"
// 	);
struct testbench::intern {
	template <class Arg, class...Args> 
	static std::string write_to_stream( std::stringstream&, Arg, Args... );
	
	template <class Arg> 
	static std::string write_to_stream( std::stringstream&, Arg );
	
	template <class...Args> 
	static std::string concatenate( Args...args );
};


//------------------------------------- IMPLEMENTATION --------------------------------------------

//
// testbench
//
testbench::testbench( const std::string& name ) 
: m_name{name} {

}

const std::string& testbench::name() const {
	return m_name;
}

testbench::testcase testbench::create( const std::string& name ) {
	return testcase(name,this);
}

int testbench::testcases() const {
	return m_logs.size();
}

int testbench::failed_testcases() const {
	int result{0};
	for(auto& l : m_logs) {
		if ( l.entries.size() )
			result++;
	}
	return result;
}

int testbench::checks() const {
	int result{0};
	for( auto& l : m_logs ) {
		result += l.check_count;
	}
	return result;
}

int testbench::failed_checks() const {
	int result{0};
	for(auto& l : m_logs) {
		result += l.entries.size();	
	}
	return result;
}

const std::vector<testbench::log>& testbench::logs() const {
	return m_logs;
}

void testbench::add( testbench::log&& l ) {
	m_logs.push_back( std::move(l) );
}

//
// testbench::testcase
//

// s - If the operation threw an exception, the corresponding message is written back.
// c - The callable object, which has to have the return type T.
template <class Callable>
void testbench::testcase::encapsulate_comparison( std::string&& s, Callable&& c ) {
	std::string msg = std::move(s);
	bool op_result;
	bool success{false};
	try {
		op_result = c();
		success = true;
	} 
	catch( std::exception& e ) {
		msg = intern::concatenate(
			"std::exception: [",
			e.what(),
			"]");
	}
	catch( ... ) {
		msg = std::string("exception (unknown type).");
	}	
	m_log.add( !success || !op_result, msg );

}



testbench::testcase::testcase( const std::string& name, testbench* parent ) 
: m_log(name), m_parent{parent} {
	
}

testbench::testcase::~testcase() {
	m_parent->add( std::move(m_log) );
}

template <class T>
void testbench::testcase::check( const T& t ) {
	m_log.add( !static_cast<bool>(t), "Expression evaluated to 'false'.");
}

template <class T>
void testbench::testcase::equal( const T& a, const T& b ) {
	encapsulate_comparison( 
		intern::concatenate("Expected [", b, "], but found [", a, "]." ),
		[&](){
			return (a==b);
		}
	);
}

template <class T>
void testbench::testcase::equal( const T& a, const T& b, const T& th ) {
	T diffabs = std::abs(a-b);
	encapsulate_comparison(
		intern::concatenate(
			"Expected [", b,"], but found [", a,
			"]. Absolute deviation=", diffabs," exceeds threshold=", th),
		[&](){
			return !(diffabs > th);
		}
	);
}

template <class T>
void testbench::testcase::less_than( const T& a, const T& b ) {
	encapsulate_comparison(
		intern::concatenate("Expected value to be less than [",b,"], but found [",a,"]."),
		[&](){
			return a < b;
		}
	);
}

template <class T>
void testbench::testcase::less_than_or_equal( const T& a, const T& b ) {
	encapsulate_comparison(
		intern::concatenate( "Expected value to be less than or equal to [",b,
			"], but found [",a,"]."),
		[&](){
			return a <= b;
		}
	);
}

template <class T>
void testbench::testcase::greater_than( const T& a, const T& b ) {
	encapsulate_comparison(
		intern::concatenate( "Expected value to be greater than [",b,
			"], but found [",a,"]."),
		[&](){
			return a > b;
		}
	);
}

template <class T>
void testbench::testcase::greater_than_or_equal( const T& a, const T& b ) {
	encapsulate_comparison( 
		intern::concatenate( "Expected value to be greater than or equal to [",b,
			"], but found [",a,"]."),
		[&](){
			return a >= b;
		}
	);
}

template <class T>
void testbench::testcase::in_range( const T& a, const T& lo, const T& hi ) {
	encapsulate_comparison(
		intern::concatenate("Expected value in [",lo,", ",hi,"], but found [",a,"]."),
		[&](){
			return ( a >= lo && a <= hi );
		}
	);
}

template <class T>
void testbench::testcase::not_in_range( const T& a, const T& lo, const T& hi ) {
	encapsulate_comparison(
		intern::concatenate("Expected value to be less that [",lo,"] or greater than [",
			hi,"], but found [",a,"]."),
		[&](){
			return ( a < lo || a > hi );
		}
	);
}

template <class Callable>
void testbench::testcase::does_not_throw( Callable&& callable ) {
	static const std::string msg1("Exception should not be thrown, but caught ");
	std::string msg2;
	bool thrown{true};
	try {
		callable();
		thrown = false;
	}
	catch( std::exception& e ) {
		msg2 = std::string(e.what());
	}
	catch( ... ) {
		msg2 = std::string("exception of unknown type.");
	}
	m_log.add( thrown , msg1+msg2 );
}

// Problem: When template parameter Exception is of type std::exception, the compiler warns 
// 	about the exception be caught earlier, in the catch-statement before.
// 	I don't want to turn warnings off. The library should compile and work without any sharp
// 	edges to it.
//
// Solution attempts:
// - Partial template specialization (Exception=std::std::exception), but it's not allowed.
// 	A full template specialization seems not possible because of the template parameter 
// 	'Callable'.
// - Used the approach of [if constexpr std::is_same<Exception,std::exception>::value], but
// 	this requires C++17, which I don't want to enforce at the moment.
//
// Current solution:
// - If it's irrelevant what type of exception is thrown, use 'throws_any'.
// - If *any* std::exception is expected, use 'throws_stdexcept'
// - If one specific exception type, which can also be a derivative of std::exception, is expected,
// 	then use 'throws', e.g. 'throws<std::range_error>'.
template <class Exception, class Callable>
void testbench::testcase::throws( Callable&& callable ) {
	static_assert( !std::is_same<Exception,std::exception>::value, 
		"Sorry for the inconvenience, but may I refer you to 'throws_stdexcept()' instead!"
	);
	std::string msg;
	bool failed{true};
	try {
		callable();
		msg = std::string("No exception has been raised.");
	}
	catch( Exception& e ) {
		failed = false;
	}
	catch( std::exception& e ) {
		msg = intern::concatenate(
			"Expected a different exception type. Caught: ", 
			e.what()
		);
	}
	catch( ... ) {
		msg = std::string("Expected a different exception type. Caught unknown type.");
	}
	m_log.add( failed, msg );
}

template <class Callable>
void testbench::testcase::throws_stdexcept( Callable&& callable ) {
	std::string msg;
	bool failed{true};
	try {
		callable();
		msg = std::string("No exception has been raised.");
	}
	catch( std::exception& e ) {
		failed = false;
	}
	catch( ... ) {
		msg = std::string("Caught unknown exception (not derived from std::exception)");
	}
	m_log.add( failed, msg );
}

template <class Callable>
void testbench::testcase::throws_any( Callable&& callable ) {
	std::string msg;
	bool failed{true};
	try {
		callable();
		msg = std::string("No exception has been raised.");
	}
	catch( ... ) {
		failed = false;
	}
	m_log.add( failed, msg );
}

//
// testbench::log 
// testbench::log::entry
//

testbench::log::log( const std::string& s )
: name{s}, check_count{0} {

}

void testbench::log::add( bool failed, const std::string& msg ) {
	check_count++;
	if ( failed ) 
		entries.push_back( entry( check_count, msg ) );
}

testbench::log::entry::entry( int pos, std::string msg )
: position{pos}, message{msg} {

}

//
// testbench::intern
//
template <class Arg>
std::string testbench::intern::write_to_stream( std::stringstream& ss, Arg last ) {
	ss << last;
	return std::string( ss.str() );
}

template <class Arg, class... Args>
std::string testbench::intern::write_to_stream( std::stringstream& ss, Arg current, Args... rest ){
	ss << current;
	return write_to_stream( ss, rest... );
}

template <class...Args>
std::string testbench::intern::concatenate( Args... args ) {
	try {
		std::stringstream ss;
		return write_to_stream( ss, args... );
	} 
	catch( ... ) {

	}
	return std::string("[Creating feedback message failed due to an exception]");
}

//
// free functions
//
std::ostream& operator<<( std::ostream& os, const testbench& tb ) {
	static const std::string Indent("           ");
	static const std::string Failed("[FAILED]   ");
	static const std::string Passed("[OK]       ");
	static const std::string Warning("[WARNING]  ");
	bool failed{false};
	os << tb.name() << '\n';
	for( auto i{ tb.name().size() }; i > 0; i-- )
		os << '-';
	os << '\n';
	auto& logs{ tb.logs() };
	for ( auto& l : logs ) {
		if ( l.entries.size() ) {
			os << Failed;
			failed = true;
		}
		else if (!l.check_count)
			os << Warning;
		else
			os << Passed;
		os << '\"' << l.name << "\" (checks: " << l.check_count << ')';
		if ( !l.check_count ) 
			os << " Empty testcase!";
		os << '\n';
		for( auto& e : l.entries ) {
			os << Indent << "#" << e.position << ": " << e.message << '\n';
		}
	}
	os << '\n';
	if ( !tb.logs().size() )
		os << "Nothing's been tested.\n";
	else {
		if ( !failed ) 
			os << "PASSED\n------\n(total: "
				<< tb.testcases() << " testcases, "
				<< tb.checks() << " checks)\n";
		else {
			os << "FAILED\n------\n" 
				<< tb.failed_testcases() << "/" << tb.testcases() 
				<< " testcases\n" 
				<< tb.failed_checks() << "/" << tb.checks() 
				<< " checks\n";
		}
	}
	return os;
}

} // namespace elrat 

#endif // include guard
