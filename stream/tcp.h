#ifndef _HTTP_STREAM_TCP_H_
#define _HTTP_STREAM_TCP_H_
#include <http_server/stream/basic.h>

namespace http {
namespace stream {

template <
    typename Socket                 = asio::ip::tcp::socket
  , typename Handler                = boost::function<void (
                                        sys::error_code const&, std::size_t)>
  , typename MutableBuffer          = asio::mutable_buffer
  , typename ConstBuffer            = asio::const_buffer
  , typename MutableBufferSequence  = std::vector<MutableBuffer>
  , typename ConstBufferSequence    = std::vector<ConstBufferSequence>
>
class basic_tcp
  : public basic<Handler, MutableBuffer, ConstBuffer, 
                 MutableBufferSequence, ConstBufferSequence>
{
public:
  typedef          Socket               socket_type;
  typedef typename basic::handler_type  handler_type;
  typedef typename basic::mutable_buffer_sequence_type 
                                        mutable_buffer_sequence_type;
  typedef typename basic::const_buffer_sequence_type   
                                        const_buffer_sequence_type;

public:
	basic_tcp (socket_type& socket) : socket_ (socket) {}
	virtual ~basic_tcp () {}

protected:
  virtual void _async_read_some (
      mutable_buffer_sequence_type const& buffers,
      handler_type handler)
  {
  	socket_.async_read_some (buffers, handler);
  }

  virtual std::size_t _async_read_some (
      mutable_buffer_sequence_type const& buffers,
      asio::yield_context yield) 
  {
  	return socket_.async_read_some (buffers, yield);
  }

  virtual void _async_write_some (
      const_buffer_sequence_type const& buffers,
      handler_type handler) 
  {
  	socket_.async_write_some (buffers, handler);
  }

  virtual std::size_t _async_write_some (
      const_buffer_sequence_type const& buffers,
      asio::yield_context yield)
  {
  	return socket_.async_write_some (buffers, yield);
  }

private:
  socket_type& socket_;
};

}} // namespace http::stream
#endif // _HTTP_STREAM_TCP_H_
