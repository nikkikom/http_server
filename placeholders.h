#ifndef _HTTP_PLACEHOLDERS_H_
#define _HTTP_PLACEHOLDERS_H_

#include <http_server/compat.h>

#include <http_server/method.h>
#include <http_server/uri/parts.h>

namespace http {

_inline_ns namespace placeholders {

namespace url {

struct tag {};

namespace {

struct tag_user_info : tag
{
	template <typename Iterator>
	boost::iterator_range<Iterator>
	operator() (method, boost::iterator_range<Iterator> const&, 
	    uri::parts<Iterator> const& parsed) const
	{
		return parsed.hier_part.user_info;
  }
} user_info;

struct tag_path : tag
{
	template <typename Iterator>
	boost::iterator_range<Iterator>
	operator() (method, boost::iterator_range<Iterator> const& path, 
	    uri::parts<Iterator> const&) const
	{
		return path;
  }
} path;

// TODO: TBD
struct tag_host {}      host;
struct tag_port {}      port;

struct tag_scheme {}    scheme;
struct tag_query {}     query;
struct tag_fragment {}  fragment;

struct tag_query_key {} key;
struct tag_query_value {} value;
} // namespace

} // namespace url

} // namespace placeholders

}

#endif // _HTTP_PLACEHOLDERS_H_
