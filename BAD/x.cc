#include <iostream>

struct A 
{
	A (int) {}
#if 0
	A (const A&) {}
	A (A&&) {}
#endif
};

template <typename T>
struct B : T
{
	using T::T;

#if 1
	B (T&& t) : T (std::move (t)) {}
	B (T const& t) : T (t) {}
#endif
};

template <typename T>
B<T> make_B (T&& t)
{
	return B<T> (std::forward<T> (t));
}

template <typename T, typename ...Args>
B<T> make_B (Args&& ...args)
{
	return B<T> (std::forward<Args> (args)...);
}

int main ()
{
	auto b = make_B<A> (5);
	auto c = make_B ([] {});
}
