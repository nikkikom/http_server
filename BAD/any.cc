#include <boost/any.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/type_index.hpp>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <utility>

#include <http_server/detail/repeat_until.h>

using boost::container::stable_vector;

#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/typeof/typeof.hpp>

#include <http_server/detail/repeat_until.h>

struct A {};
struct B {};
struct C {};

template <typename Sig>
struct handler_base
{
#if 1
	handler_base () {}
	handler_base (handler_base const&) = delete;
	handler_base (handler_base&&x) : q_ (std::move (x.q_)) {}

	typedef typename stable_vector<std::function<Sig> >::iterator iterator;
	stable_vector<std::function<Sig> > q_;
	
	template <typename F>
	iterator insert (F&& f)
	{
		return q_.insert (q_.end (), std::forward<F> (f));
  }
#endif
};

template <class H> class foo_B_A : handler_base<bool (B)> 
{
  H handler_;
public:
  template <class> struct result {};
  template <class F> struct result<F(B)> { typedef bool type; };

  // foo_B_A (foo_B_A const&) = delete;

  foo_B_A (H&& h) : handler_ (std::forward<H> (h)) {}
  bool operator() (B) const { return handler_ (A ()); }
};

template <class H> class foo_C_B : handler_base<bool (C)> 
{
  H handler_;
public:
  template <class> struct result {};
  template <class F> struct result<F(C)> { typedef bool type; };
  // foo_C_B (foo_C_B const&) = delete;
  foo_C_B (H&& h) : handler_ (std::forward<H> (h)) {}
  bool operator() (C) const { return handler_ (B ()); }
};

template <typename H>
foo_B_A<H>
foo (H&& h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(A)>::type, bool> >::type* = 0)
{
  return foo_B_A<H> (std::forward<H> (h));
}

template <typename H>
foo_C_B<H>
foo (H&& h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(B)>::type, bool> >::type* = 0)
{
  return foo_C_B<H> (std::forward<H> (h));
}

struct bad_type {};
template <class, class=void> struct foo_return_traits { typedef bad_type type; };

template <class F>
struct foo_return_traits<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F(A)>::type, bool> >::type>
{
  typedef foo_B_A<F> type;
};

template <class F>
struct foo_return_traits<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F(B)>::type, bool> >::type>
{
  typedef foo_C_B<F> type;
};

struct foo_functor
{
  template <class> struct result {};
  template <class F, class H> struct result<F(H)>
  {
    typedef typename foo_return_traits<H>::type type;
    static_assert(!boost::is_same<type, bad_type>::value,
      "Cannot find handler with such signature");
  };

  template <typename H>
  // auto
  typename foo_return_traits<H>::type
  operator() (H&& h) const 
    // -> decltype (foo (std::forward<H> (h)))
  { return foo (std::forward<H> (h)); }
};

#if 0
struct handler_int: handler_base<bool (int)>
{
	int i_;

	void prepare (int i,char) { i_ = i; std::cout << "iprepare: " << i_ << "\n";}

	bool operator() (iterator iter)
	{
		return (*iter) (i_);
  }
};

struct handler_char: handler_base<bool (char)>
{
	char i_;

	void prepare (int,char i) { i_ = i; std::cout << "prepare: " << i_ << "\n"; }

	bool operator() (iterator iter)
	{
		return (*iter) (i_);
  }
};
  
handler_int hi;
handler_char hc;
#endif
using boost::typeindex::type_index;

std::map<type_index, boost::any> handler_map;

template <class Sig, class Action, class Arg>
typename boost::result_of<
  http::detail::repeat_until_helper<Action,Sig,Arg> (Action,Arg)
>::type&
get_handler( Action&& action, Arg&& arg )
{
  using namespace http::detail;

  typedef typename boost::result_of<
    repeat_until_helper<Action,Sig,Arg> (Action,Arg)
  >::type result_type;

  type_index tid = boost::typeindex::type_id<result_type> ();

  if (handler_map.find (tid) == handler_map.end ())
  {
  	// handler_map.insert (decltype(handler_map)::value_type (tid,
  	 repeat_until<Sig> (std::forward<Action> (action),
  	   std::forward<Arg> (arg))
    // ));
    ;
  }

 	return boost::any_cast<result_type&> (handler_map[tid]);
}


  std::vector<std::pair<
    std::function<void (int,char)>,
    std::function<void ()>
  > > functors;

  std::set<void*> registry;

template <typename X, typename L>
void reg (X& x, L&& l)
{
  static auto const do_nothing = [] (int,char) {};
  if (registry.find (&x) == registry.end ())
  {
    functors.push_back (decltype (functors)::value_type (
      [&x] (int i, char c) { x.prepare (i,c); },
      [&x, iter=x.insert (std::forward<L> (l))] { x (iter); }
    ));

    registry.insert (&x);
  }
  else
  {
    functors.push_back (decltype (functors)::value_type (
      do_nothing,
      [&x, iter=x.insert (std::forward<L> (l))] { x (iter); }
    ));
  }
}

template <class, class=void> struct rreg_t {};

template <class F>
struct rreg_t<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F (C)>::type, bool> >::type>
{ typedef bool type (C); };

template <class F>
struct rreg_t<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F (B)>::type, bool> >::type>
{ typedef bool type (B); };

template <class F>
void rreg (F l)
{
  typedef typename rreg_t<F>::type ftype;
  // auto& h = 
#if 0
   get_handler<ftype> (foo_functor (), 
    [] (A) { std::cout << "inner handler\n"; return true; });

  reg (h, l);
#endif
}

int main ()
{
	auto l1 = [] (C) { std::cout << "(C) " << "\n"; return true; };
	auto l2 = [] (B) { std::cout << "(B) " << "\n"; return true; };

#if 0
  auto& h = get_handler<bool (C)> (foo_functor (), 
    [] (A) { std::cout << "inner handler\n"; return true; });

  h (C{});

  reg (hc, l2);
  reg (hi, l1);
  reg (hi, l1);
#endif
	//rreg (l1);

  auto& h = get_handler<bool (C)> (foo_functor (), 
    [] (A) { std::cout << "inner handler\n"; return true; });

  for (auto& x : functors) 
  {
  	x.first (55, 'x');
  	x.second ();
  }

}
