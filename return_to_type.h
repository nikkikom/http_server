#ifndef _HTTP_RETURN_TO_TYPE_H_
#define _HTTP_RETURN_TO_TYPE_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>

#if __cplusplus < 201103L
# include <boost/preprocessor.hpp>
# include <boost/move/move.hpp>
#endif

#include <utility>

namespace http {
namespace detail {

template <typename Handler, typename R>
class return_to_type 
{
public:
  typedef R result_type;

#if __cplusplus >= 201103L
  template <typename H>
	return_to_type (H&& handler, R err,
	      typename boost::enable_if<boost::is_same<Handler,H>>::type* = 0) 
	  : handler_ (std::move (handler))
	  , default_error_ (err)
	{}

  template <typename ...Args>
	R operator() (Args&& ...args)
	{
		return convert<Handler> (std::forward<Args> (args)...);
  }
#else
	return_to_type (Handler handler, R err) 
	  : handler_ (boost::move (handler))
	  , default_error_ (err)
	{}

	R operator() () { return convert<Handler> (); }
	R operator() () const { return convert<Handler> (); }

#define LIMITS (2, 2)
#define TEXT(z, n, text) BOOST_PP_CAT(text,n)
#define TEXT2(z, n, text) BOOST_PP_CAT(A,n) BOOST_PP_CAT(a,n)

#define SAMPLE(n) \
  template <BOOST_PP_ENUM(n, TEXT, class A)> \
  R operator() (BOOST_PP_ENUM(n, TEXT2, ~)) \
  { return convert<Handler> (BOOST_PP_ENUM(n, TEXT, a)); } \
  template <BOOST_PP_ENUM(n, TEXT, class A)> \
  R operator() (BOOST_PP_ENUM(n, TEXT2, ~)) const \
  { return convert<Handler> (BOOST_PP_ENUM(n, TEXT, a)); } \

#define BOOST_PP_LOCAL_LIMITS LIMITS
#define BOOST_PP_LOCAL_MACRO(n) SAMPLE(n)

%:include BOOST_PP_LOCAL_ITERATE()

#undef LIMITS
#undef TEXT
#undef TEXT2
#undef SAMPLE

#endif

protected:
#if __cplusplus >= 201103L
  template <typename H, typename ...Args>
  typename boost::enable_if<
      boost::is_same<typename boost::result_of<H (Args...)>::type, R>,
      R
  >::type
  convert (Args&& ...args) 
  {
    return handler_ (std::forward<Args> (args)...);
  }

  template <typename H, typename ...Args>
  typename boost::enable_if<
      boost::is_same<typename boost::result_of<H (Args...)>::type, bool>,
      R
  >::type
  convert (Args&& ...args) 
  {
    return handler_ (std::forward<Args> (args)...) ? R () : default_error_;
  }

  template <typename H, typename ...Args>
  typename boost::enable_if<
      boost::is_same<typename boost::result_of<H (Args...)>::type, void>,
      R
  >::type
  convert (Args&& ...args) 
  {
    handler_ (std::forward<Args> (args)...);
    return R ();
  }
#else
  //////////////////////////////////////////////////////////////////////////////
  // return type is R
	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, R>,
	  R
	>::type convert () { return handler_ (); }

	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, R>,
	  R
	>::type convert () const { return handler_ (); }

#define LIMITS (2, 2)
#define TEXT(z, n, text) BOOST_PP_CAT(text,n)
#define TEXT2(z, n, text) BOOST_PP_CAT(A,n) BOOST_PP_CAT(a,n)

#define SAMPLE(n) \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, R>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) \
	{ return handler_ (BOOST_PP_ENUM(n, TEXT, a)); } \
  \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, R>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) const \
	{ return handler_ (BOOST_PP_ENUM(n, TEXT, a)); }

#define BOOST_PP_LOCAL_LIMITS LIMITS
#define BOOST_PP_LOCAL_MACRO(n) SAMPLE(n)

%:include BOOST_PP_LOCAL_ITERATE()

#undef LIMITS
#undef TEXT
#undef TEXT2
#undef SAMPLE

  //////////////////////////////////////////////////////////////////////////////
  // return type is bool
	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, bool>,
	  R
	>::type convert () 
	{ if (handler_ ()) return R (); else return default_error_; }

	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, bool>,
	  R
	>::type convert () const
	{ if (handler_ ()) return R (); else return default_error_; }

#define LIMITS (2, 2)
#define TEXT(z, n, text) BOOST_PP_CAT(text,n)
#define TEXT2(z, n, text) BOOST_PP_CAT(A,n) BOOST_PP_CAT(a,n)

#define SAMPLE(n) \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, bool>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) \
	{ if (handler_ (BOOST_PP_ENUM(n, TEXT, a))) return R (); \
	  else return	default_error_; } \
  \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, bool>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) const \
	{ if (handler_ (BOOST_PP_ENUM(n, TEXT, a))) return R (); \
	  else return	default_error_; } 

#define BOOST_PP_LOCAL_LIMITS LIMITS
#define BOOST_PP_LOCAL_MACRO(n) SAMPLE(n)

%:include BOOST_PP_LOCAL_ITERATE()

#undef LIMITS
#undef TEXT
#undef TEXT2
#undef SAMPLE

  //////////////////////////////////////////////////////////////////////////////
  // return type is void
	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, void>,
	  R
	>::type convert () 
	{ handler_ (); return R (); }

	template <class H>
	typename boost::enable_if<
	  boost::is_same<typename boost::result_of<H ()>::type, void>,
	  R
	>::type convert () const
	{ handler_ (); return R (); }

#define LIMITS (2, 2)
#define TEXT(z, n, text) BOOST_PP_CAT(text,n)
#define TEXT2(z, n, text) BOOST_PP_CAT(A,n) BOOST_PP_CAT(a,n)

#define SAMPLE(n) \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, void>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) \
	{ handler_ (BOOST_PP_ENUM(n, TEXT, a)); return R (); } \
  \
	template <class H, BOOST_PP_ENUM(n, TEXT, class A)> \
	typename boost::enable_if< \
	  boost::is_same<typename boost::result_of< \
	    H (BOOST_PP_ENUM(n, TEXT, A)) \
	  >::type, void>, \
	  R \
	>::type convert (BOOST_PP_ENUM(n, TEXT2, ~)) const \
	{ handler_ (BOOST_PP_ENUM(n, TEXT, a)); return R (); } 

#define BOOST_PP_LOCAL_LIMITS LIMITS
#define BOOST_PP_LOCAL_MACRO(n) SAMPLE(n)

%:include BOOST_PP_LOCAL_ITERATE()

#undef LIMITS
#undef TEXT
#undef TEXT2
#undef SAMPLE

#endif

private:
  Handler handler_;
  R default_error_;
};

} // namespace detail

#if __cplusplus >= 201103L
template <typename Handler, typename R>
detail::return_to_type<Handler, R>
return_to_type (Handler&& handler, R err)
{
	return detail::return_to_type<Handler,R> (
	  std::forward<Handler> (handler), err
	);
}
#else
template <typename Handler, typename R>
detail::return_to_type<Handler,R>
return_to_type (Handler handler, R err)
{
	return detail::return_to_type<Handler,R> (boost::move (handler), err);
}
#endif

 
} // namespace http
#endif // _HTTP_RETURN_TO_TYPE_H_
