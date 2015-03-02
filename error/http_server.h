#ifndef _HTTP_SERVER_ERROR_HTTP_SERVER_H_
#define _HTTP_SERVER_ERROR_HTTP_SERVER_H_

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace http { 
namespace error {

enum http_server_errors {
	no_request_handler = 1,
};

namespace detail {
class http_server_category: public boost::system::error_category
{
public:
  const char* name () const BOOST_SYSTEM_NOEXCEPT
  {
  	return "http.server";
  }

  std::string message (int value) const
  {
  	if (value == error::no_request_handler)
      return "No suitable request handler";

    return "http.server error";
  }
};
} // namespace detail

inline const boost::system::error_category&
get_http_server_category ()
{
	static detail::http_server_category instance;
	return instance;
}

static const boost::system::error_category& http_server_category
  = get_http_server_category ();

}} // namespace http::error

namespace boost {
namespace system {

template <> struct is_error_code_enum<http::error::http_server_errors>
{
	static const bool value = true;
};

}} // boost::system

namespace http {
namespace error {

inline boost::system::error_code make_error_code (http_server_errors e)
{
	return boost::system::error_code (
	  static_cast<int> (e), get_http_server_category ());
}

}} // namespace http::error
#endif // _HTTP_SERVER_ERROR_HTTP_SERVER_H_
