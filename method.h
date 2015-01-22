#ifndef _HTTP_METHOD_H_
#define _HTTP_METHOD_H_
namespace http {

#if __cplusplus < 201103L
# define _enum_class
#else
# define _enum_class class
#endif

enum _enum_class method {
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

} // namespace http
#endif // _HTTP_METHOD_H_
