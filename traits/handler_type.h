#ifndef _HTTP_TRAITS_HANDLER_TYPE_H_
#define _HTTP_TRAITS_HANDLER_TYPE_H_
#include <boost/mpl/bool.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <utility> // std::forward

namespace http { namespace traits {
namespace mpl = ::boost::mpl;

namespace detail {

template <typename T>
struct handler_stream_helper: T
{
	using T::T;

	handler_stream_helper (T&& t) : T (std::forward<T> (t)) {}
	handler_stream_helper (T const& t) : T (t) {}
};

template <typename T>
struct handler_asio_buffer_helper: T
{
	using T::T;

	handler_asio_buffer_helper (T&& t) : T (std::forward<T> (t)) {}
	handler_asio_buffer_helper (T const& t) : T (t) {}
};

template <typename T>
struct is_handler_stream: mpl::false_ {};

template <typename T>
struct is_handler_stream<detail::handler_stream_helper<T> >: mpl::true_ {};

template <typename T, class ResultOf = void>
struct is_handler_stream_signature: mpl::false_ {};

template <typename T>
struct is_handler_stream_signature<T, typename boost::result_of<T (int)>::type>
: mpl::true_ {};

template <typename T>
struct is_handler_asio_buffer: mpl::false_ {};

template <typename T>
struct is_handler_asio_buffer<detail::handler_asio_buffer_helper<T> >
    : mpl::true_ {};

template <typename T, class Enable = void>
struct is_handler_asio_buffer_signature: mpl::false_ {};

template <typename T>
struct is_handler_asio_buffer_signature<T, 
    typename boost::result_of<T (char const*)>::type> : mpl::true_ {};

template <typename T, class Enable = void>
struct is_handler_other_signature: mpl::false_ {};

template <typename T>
struct is_handler_other_signature<T, typename boost::result_of<T ()>::type>
  : mpl::true_ {};

template <typename T>
struct nake_handler_helper { typedef T type; };

template <typename T>
struct nake_handler_helper<handler_stream_helper<T> > 
{ typedef T type; };

template <typename T>
struct nake_handler_helper<handler_asio_buffer_helper<T> > 
{ typedef T type; };

} // namespace detail

template <typename T>
struct nake_handler
{
	typedef typename detail::nake_handler_helper<
	          typename detail::nake_handler_helper<T>::type
	>::type type;
};

#if __cplusplus >= 201103L
template <typename T> using nake_handler_t = typename nake_handler<T>::type;
#endif

template <typename T>
struct is_handler_stream: std::conditional<
   detail::is_handler_stream<T>::value  
  || (! detail::is_handler_asio_buffer<T>::value 
    && detail::is_handler_stream_signature<T>::value
  )
  , mpl::true_, mpl::false_>::type {};

template <typename T>
struct is_handler_asio_buffer: std::conditional<
   detail::is_handler_asio_buffer<T>::value  
  || (! detail::is_handler_stream<T>::value 
    && detail::is_handler_asio_buffer_signature<T>::value
  )
  , mpl::true_, mpl::false_>::type {};

template <typename T>
struct is_handler_other: std::conditional<
     ! detail::is_handler_stream<T>::value
  && ! detail::is_handler_asio_buffer<T>::value
  &&   detail::is_handler_other_signature<T>::value 
  , mpl::true_, mpl::false_>::type {};

template <typename T>
detail::handler_stream_helper<typename nake_handler<T>::type>
handler_stream (T&& t)
{
  return detail::handler_stream_helper<typename nake_handler<T>::type> (
	    std::forward<T> (t));
}

template <typename T, typename... Args>
detail::handler_stream_helper<typename nake_handler<T>::type>
handler_stream (Args&& ...args)
{
  return detail::handler_stream_helper<typename nake_handler<T>::type> (
	    std::forward<Args> (args)...);
}

template <typename T>
detail::handler_asio_buffer_helper<typename nake_handler<T>::type>
handler_asio_buffer (T&& t)
{
  return detail::handler_asio_buffer_helper<typename nake_handler<T>::type> (
	    std::forward<T> (t));
}

template <typename T, typename... Args>
detail::handler_asio_buffer_helper<typename nake_handler<T>::type>
handler_asio_buffer (Args&& ...args)
{
  return detail::handler_asio_buffer_helper<typename nake_handler<T>::type> (
	    std::forward<Args> (args)...);
}

template <typename T>
struct is_handler_stream<detail::handler_stream_helper<T> >: mpl::true_
{};

}} // http::traits
#endif // _HTTP_TRAITS_HANDLER_TYPE_H_
