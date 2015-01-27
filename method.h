#ifndef _HTTP_METHOD_H_
#define _HTTP_METHOD_H_
namespace http {

#if __cplusplus < 201103L
# define _enum_class
# define HttpMethod method::method_
#else
# define _enum_class class
# define HttpMethod method
#endif

#if __cplusplus < 201103L
struct method { enum method_ {
#else
enum _enum_class method {
#endif
	Options,
  Get,
  Head,
  Post,
  Put,
  Patch,
  Delete,
  Trace,
  Connect,

  Unknown,
};
#if __cplusplus < 201103L
};
#endif

} // namespace http
#endif // _HTTP_METHOD_H_
