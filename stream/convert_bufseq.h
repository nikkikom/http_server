#ifndef _HTTP_STREAM_CONVERT_BUFSEQ_H_
#define _HTTP_STREAM_CONVERT_BUFSEQ_H_

#if __cplusplus >= 201103L
#include <boost/mpl/or.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/detail/enabler.h>
#endif

namespace http {
namespace stream {

#if __cplusplus >= 201103L
namespace detail {

template <typename T>
struct is_reference_or_const: ::boost::mpl::or_<
    ::boost::is_reference<T>, ::boost::is_const<T>
> {};

}

// 1. pass the very same name, no convertion is needed.
template <typename Target, typename Range>
Target 
convert_bufseq (Ragne&& range, typename boost::enable_if<
  boost::is_same<typename boost::decay<Range>::type, Target>, 
      detail::enabler>::type = detail::enabler ())
{
	return Target (std::forward<Range> (range));
}

// 2. different container, passed by reference or const rvalue
template <typename Target, typename Range>
Target 
convert_bufseq (Ragne&& range, typename boost::disable_if<
  boost::is_same<typename boost::decay<Range>::type, Target>, 
      detail::enabler>::type = detail::enabler (),

  // container passed as const or reference (so, it cannot be moved)
  typename boost::enable_if<detail::is_reference_or_const<Range>, 
      detail::enabler>::type = detail::enabler ())
{
	using namespace std;
	using namespace boost;

	return Target (begin (s), end (s));
}

// 3. different container, passed by mutable rvalue
template <typename Target, typename Range>
Target 
convert_bufseq (Ragne&& range, typename boost::disable_if<
  boost::is_same<typename boost::decay<Range>::type, Target>, 
      detail::enabler>::type = detail::enabler (),

  // container passed as const or reference (so, it cannot be moved)
  typename boost::disable_if<detail::is_reference_or_const<Range>, 
      detail::enabler>::type = detail::enabler ())
{
	using namespace std;
	using namespace boost;
  
	return Target (make_move_iterator (begin (s)), make_move_iterator (end (s)));
}
#else
template <typename Target, typename Range>
inline Target 
convert_bufseq (Ragne const& range)
{
	return Target (range);
}
#endif

}} // namespace http::stream
#endif // _HTTP_STREAM_CONVERT_BUFSEQ_H_
