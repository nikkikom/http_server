#ifndef _HTTP_DETAIL_SIGNATURE_TO_VECTOR_H_
#define _HTTP_DETAIL_SIGNATURE_TO_VECTOR_H_

#include <boost/mpl/vector.hpp>

// Convert function signature into mpl vector
namespace http { namespace detail {

template <class> struct signature_to_vector;

template <class R> struct signature_ret_base
{
	typedef R return_type;
};

template <class R>
struct signature_to_vector<R ()>: signature_ret_base<R>
{
	typedef boost::mpl::vector<> args_type;
};

template <class R, class A0>
struct signature_to_vector<R (A0)>: signature_ret_base<R>
{
	typedef boost::mpl::vector<A0> args_type;
};

template <class R, class A0, class A1>
struct signature_to_vector<R (A0,A1)>: signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1> args_type;
};

template <class R, class A0, class A1, class A2>
struct signature_to_vector<R (A0,A1,A2)>: signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2> args_type;
};

template <class R, class A0, class A1, class A2, class A3>
struct signature_to_vector<R (A0,A1,A2,A3)>: signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4>
struct signature_to_vector<R (A0,A1,A2,A3,A4)>: signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4,
          class A5>
struct signature_to_vector<R (A0,A1,A2,A3,A4,A5)>
  : signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4,A5> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4,
          class A5, class A6>
struct signature_to_vector<R (A0,A1,A2,A3,A4,A5,A6)>
  : signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4,A5,A6> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4,
          class A5, class A6, class A7>
struct signature_to_vector<R (A0,A1,A2,A3,A4,A5,A6,A7)>
  : signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4,A5,A6,A7> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4,
          class A5, class A6, class A7, class A8>
struct signature_to_vector<R (A0,A1,A2,A3,A4,A5,A6,A7,A8)>
  : signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4,A5,A6,A7,A8> args_type;
};

template <class R, class A0, class A1, class A2, class A3, class A4,
          class A5, class A6, class A7, class A8, class A9>
struct signature_to_vector<R (A0,A1,A2,A3,A4,A5,A6,A7,A8,A9)>
  : signature_ret_base<R>
{
	typedef boost::mpl::vector<A0,A1,A2,A3,A4,A5,A6,A7,A8,A9> args_type;
};

}} // namespace http::detail

#endif // _HTTP_DETAIL_SIGNATURE_TO_VECTOR_H_
