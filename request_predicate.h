#ifndef _HTTP_REQUEST_PREDICATE_H_
#define _HTTP_REQUEST_PREDICATE_H_
#include <http_server/method.h>
#include <http_server/uri/parts.h>

#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits.hpp>
#include <boost/range.hpp>
#include <boost/mpl/bool.hpp>

#include <utility>

namespace http {

namespace mpl = ::boost::mpl;

////////////////////////////////////////////////////////////////////////////////
// Full predicate: method, path, parsed_req
template <typename T, typename Iterator, typename ResultOf = bool>
struct is_full_request_predicate: mpl::false_ {};

template <typename T, typename Iterator>
struct is_full_request_predicate<T, Iterator, typename boost::result_of<T (
        method, boost::iterator_range<Iterator>, uri::parts<Iterator>
      )>::type> : mpl::true_ {};

////////////////////////////////////////////////////////////////////////////////
// Parsed only predicate: method, parsed_req
template <typename T, typename Iterator, typename ResultOf = bool>
struct is_parsed_request_predicate: mpl::false_ {};

template <typename T, typename Iterator>
struct is_parsed_request_predicate<T, Iterator, typename boost::result_of<T (
        method, uri::parts<Iterator>
      )>::type> : mpl::true_ {};

////////////////////////////////////////////////////////////////////////////////
// Path only predicate: method, path
template <typename T, typename Iterator, typename ResultOf = bool>
struct is_path_request_predicate: mpl::false_ {};

template <typename T, typename Iterator>
struct is_path_request_predicate<T, Iterator, typename boost::result_of<T (
        method, boost::iterator_range<Iterator>
      )>::type> : mpl::true_ {};


////////////////////////////////////////////////////////////////////////////////
// Method only predicate: method
template <typename T, typename Iterator, typename ResultOf = bool>
struct is_method_request_predicate: mpl::false_ {};

template <typename T, typename Iterator>
struct is_method_request_predicate<T, Iterator, typename boost::result_of<T (
        method
      )>::type> : mpl::true_ {};


////////////////////////////////////////////////////////////////////////////////
// Normalize full predicate (method, path, parsed_req)
template <typename Pred>
class normalized_full_predicate
{
public:
  normalized_full_predicate (Pred pred) : pred_ (pred) {}

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> path, 
      uri::parts<Iterator> const& parsed)
  {
  	return pred_ (m, path, parsed);
  }

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> path, 
      uri::parts<Iterator> const& parsed) const
  {
  	return pred_ (m, path, parsed);
  }

private:
	Pred pred_;
};

template <typename Iterator, typename Pred>
normalized_full_predicate<Pred>
normalize_predicate (Pred pred, typename boost::enable_if<
    is_full_request_predicate<Pred,Iterator> >::type* = 0)
{
	return normalized_full_predicate<Pred> (pred);
}

////////////////////////////////////////////////////////////////////////////////
// Normalize parsed predicate (method, parsed_req)
template <typename Pred>
class normalized_parsed_predicate
{
public:
  normalized_parsed_predicate (Pred pred) : pred_ (pred) {}

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator>, 
      uri::parts<Iterator> const& p)
  {
  	return pred_ (m, p);
  }

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator>, 
      uri::parts<Iterator> const& p) const
  {
  	return pred_ (m, p);
  }

private:
	Pred pred_;
};

template <typename Iterator, typename Pred>
normalized_parsed_predicate<Pred>
normalize_predicate (Pred pred, typename boost::enable_if<
    is_parsed_request_predicate<Pred,Iterator> >::type* = 0)
{
	return normalized_parsed_predicate<Pred> (pred);
}

////////////////////////////////////////////////////////////////////////////////
// Normalize path predicate (method, path)
template <typename Pred>
class normalized_path_predicate
{
public:
  normalized_path_predicate (Pred pred) : pred_ (pred) {}

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> const& path, 
      uri::parts<Iterator> const&)
  {
  	return pred_ (m, path);
  }

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> const& path,
      uri::parts<Iterator> const&) const
  {
  	return pred_ (m, path);
  }

private:
	Pred pred_;
};

template <typename Iterator, typename Pred>
normalized_path_predicate<Pred>
normalize_predicate (Pred pred, typename boost::enable_if<
    is_path_request_predicate<Pred,Iterator> >::type* = 0)
{
	return normalized_path_predicate<Pred> (pred);
}


////////////////////////////////////////////////////////////////////////////////
// Normalize method predicate (method)
template <typename Pred>
class normalized_method_predicate
{
public:
  normalized_method_predicate (Pred pred) : pred_ (pred) {}

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> const&, 
      uri::parts<Iterator> const&)
  {
  	return pred_ (m);
  }

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> const&,
      uri::parts<Iterator> const&) const
  {
  	return pred_ (m);
  }

private:
	Pred pred_;
};

template <typename Iterator, typename Pred>
normalized_method_predicate<Pred>
normalize_predicate (Pred pred, typename boost::enable_if<
    is_method_request_predicate<Pred,Iterator> >::type* = 0)
{
	return normalized_method_predicate<Pred> (pred);
}




} // namespace http
#endif // _HTTP_REQUEST_PREDICATE_H_
