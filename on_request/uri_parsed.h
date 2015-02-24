#ifndef _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#define _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/static_assert.hpp>
#include <boost/logic/tribool.hpp>

#include <http_server/trace.h>
#include <http_server/on_request.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

namespace http { 

namespace detail {

template <class ResultF, class Iterator, class Handler>
class on_request_uri_parsed
{
#if 1
	BOOST_STATIC_ASSERT_MSG ((boost::is_same<
	    typename boost::result_of<ResultF(bool)>::type, void>::value),
	    "Result Functor should have 'void (bool)' signature"
	);
#endif

public:
  typedef typename boost::decay<Handler>::type _Handler;

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
  template <class> struct result {};

  template <class F, class SmartSock>
  struct result<F (ResultF, SmartSock, detail::final_call_tag)>
  {
  	typedef typename boost::result_of<_Handler (
  	  ResultF, http::HttpMethod, uri::parts<Iterator>, SmartSock
  	)>::type type;

  };
#endif

#if __cplusplus >= 201103L
  template <typename H, class = typename boost::enable_if<
      boost::is_same<typename boost::decay<H>::type, _Handler>>::type>
  on_request_uri_parsed (H&& h) : handler_ (std::forward<H> (h)) {}
#else
	on_request_uri_parsed (_Handler const& h) : handler_ (h) {}
#endif

  // TODO: move version for ResultF
	template <typename SmartSock>
	typename boost::result_of<_Handler (
	  ResultF, http::HttpMethod, uri::parts<Iterator>, SmartSock
	)>::type
	operator() (ResultF result_functor, SmartSock sock, detail::final_call_tag)
	{
#if 0
		typedef typename 
		  boost::result_of<on_request_uri_parsed (
		      ResultF, SmartSock, detail::final_call_tag)>::type result_type;
#endif

		// TODO: parse method and uri::parts here ...

		return handler_ (result_functor, 
		    method::Get, uri::parts<Iterator> (), sock);
  }

private:
  _Handler handler_;
};

}

#if __cplusplus >= 201103L
template <class ResultF, class Iterator, class SmartSock, class Handler>
#if __cplusplus < 201300L
detail::on_request_uri_parsed<ResultF,Iterator,Handler>
#else
auto
#endif
on_request (Handler&& handler, typename boost::enable_if_c<
      boost::is_same<typename boost::result_of<
        typename boost::decay<Handler>::type (
              ResultF, http::HttpMethod, uri::parts<Iterator>, SmartSock
      )>::type, boost::tribool>::value
  &&  boost::is_same<typename boost::result_of<ResultF (bool)>::type, 
        void>::value
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<ResultF, Iterator, Handler> (
      std::forward<Handler> (handler));
}
#else
template <class ResultF, class Iterator, class SmartSock, class Handler>
detail::on_request_uri_parsed<ResultF,Iterator,Handler> 
on_request (Handler const& handler, typename boost::enable_if_c<
      boost::is_same<typename boost::result_of<Handler (
              ResultF, http::HttpMethod, uri::parts<Iterator>, SmartSock
      )>::type, boost::tribool>::value
#if 0
  &&  boost::is_same<typename boost::result_of<ResultF (bool)>::type, 
        void>::value
#endif
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<ResultF, Iterator, Handler> (handler);
}
#endif

#if __cplusplus < 201103L
namespace traits {

template <class ResultF, class Iterator, class SmartSock, class Handler>
struct on_request<ResultF,Iterator,SmartSock,Handler, 
  typename boost::enable_if_c<
      boost::is_same<typename boost::result_of<Handler (
        ResultF, http::HttpMethod, uri::parts<Iterator>, SmartSock
      )>::type, boost::tribool>::value
#if 0
  &&  boost::is_same<typename boost::result_of<
        ResultF (bool)>::type, void>::value
#endif
  >::type>
{
	typedef detail::on_request_uri_parsed<ResultF, Iterator, Handler> type;
};

} // namespace traits
#endif // C++03


} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_