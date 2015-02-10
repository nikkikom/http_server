#ifndef _HTTP_DETAIL_REPEAT_UNTIL_H_
#define _HTTP_DETAIL_REPEAT_UNTIL_H_

#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/typeof/typeof.hpp>

#include <http_server/detail/vector_to_signature.h>
#include <http_server/detail/signature_to_vector.h>

namespace http { namespace detail {

// See
// http://stackoverflow.com/questions/28373514/recursive-calling-overloaded-c-functions

template <class Action, class StopSig, class Arg, class = void>
struct repeat_until_helper
{
  typedef typename boost::result_of<Action (Arg)>::type action_result;

  template <class> struct result {};
  template <class F> struct result<F(Action,Arg)> {
  	typedef typename boost::result_of<
  	  repeat_until_helper<Action, StopSig, action_result>(Action, action_result)
  	>::type type;
  };

  typename boost::result_of<
    repeat_until_helper <Action, StopSig, action_result> (Action, action_result)
  >::type operator() (Action action, Arg arg) const
  {
  	return repeat_until_helper <Action, StopSig, action_result> ()
  	  (action, action (arg));
  }
};

template <typename Action, class StopSig, class Arg>
struct repeat_until_helper<Action, StopSig, Arg,
  typename boost::enable_if<
    boost::is_same<typename boost::result_of<
      typename vector_to_signature<Arg,
        typename signature_to_vector<StopSig>::args_type
      >::type
    >::type, typename signature_to_vector<StopSig>::return_type>
  >::type>
{
  template <class> struct result {};
  template <class F> struct result<F(Action,Arg)> {
    typedef Arg type;
  };

  Arg operator() (Action action, Arg arg) const { return arg; }
};


template <class StopSig, class Action, class Arg>
typename boost::result_of<
    repeat_until_helper <Action, StopSig, Arg> (Action, Arg)
>::type repeat_until (Action action, Arg arg)
{
  return repeat_until_helper<Action, StopSig, Arg> () (action, arg);
}

}} // namespace http::detail

#endif // _HTTP_DETAIL_REPEAT_UNTIL_H_
