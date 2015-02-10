#ifndef _HTTP_DETAIL_VECTOR_TO_SIGNATURE_H_
#define _HTTP_DETAIL_VECTOR_TO_SIGNATURE_H_

#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

// Convert mpl vector into function signature
namespace http { namespace detail {

template <class Ret, size_t N, class Vector>
struct vector_to_signature_;

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 0, Vector>
{
	typedef Ret type ();
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 1, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 2, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 3, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 4, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 5, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 6, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<5> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 7, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<5> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<6> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 8, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<5> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<6> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<7> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 9, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<5> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<6> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<7> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<8> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature_<Ret, 10, Vector>
{
	typedef Ret type (
      typename boost::mpl::at<Vector,boost::mpl::int_<0> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<1> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<2> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<3> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<4> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<5> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<6> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<7> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<8> >::type
    , typename boost::mpl::at<Vector,boost::mpl::int_<9> >::type
	);
};

template <class Ret, class Vector>
struct vector_to_signature 
  : vector_to_signature_<Ret, boost::mpl::size<Vector>::value, Vector> {};

}} // namespace http::detail

#endif // _HTTP_DETAIL_VECTOR_TO_SIGNATURE_H_
