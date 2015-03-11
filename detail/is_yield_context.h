#ifndef _HTTP_DETAIL_IS_YIELD_CONTEXT_H_
#define _HTTP_DETAIL_IS_YIELD_CONTEXT_H_

#if __cplusplus < 201103L
# include <boost/type_traits.hpp>
#else
# include <type_traits>
#endif

namespace http { namespace detail {
#if __cplusplus < 201103L
using ::boost::true_type;
using ::boost::false_type;
#else
using ::std::true_type;
using ::std::false_type;
#endif

template <class> struct is_yield_context: false_type {};

template <class H>
struct is_yield_context< ::boost::asio::basic_yield_context<H> > : true_type {};

}}
#endif // _HTTP_DETAIL_IS_YIELD_CONTEXT_H_
