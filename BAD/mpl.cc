#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>

#include <string>
#include <iostream>

#include <http_server/detail/signature_to_vector.h>
#include <http_server/detail/vector_to_signature.h>

struct A 
{
	void operator() (int, int);
	bool operator() (std::string);
};

#include <boost/function.hpp>

bool foo (int a, int b)
{
	std::cout << a+b << "\n";
	return true;
}

int main ()
{
	// typedef boost::mpl::apply1<_1 (int,int), A>::type F;
	using namespace http::detail;
	typedef signature_to_vector<bool (std::string)> T;

	typedef vector_to_signature<A, T::args_type>::type R;

	// boost::function<R> func = &foo;
	// func (5,4);

	boost::result_of<R>::type b = true;
}
