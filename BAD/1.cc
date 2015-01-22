#include <vector>
#include <boost/unique_ptr.hpp>
#include <boost/noncopyble.hpp>


struct A : boost::noncopyable
{
};

typedef boost::unique_ptr<A> APTR;

int main ()
{
	std::vector<APTR> va;
}
