#ifndef _HTTP_STREAM_BASIC_H_
#define _HTTP_STREAM_BASIC_H_

#include <http_server/asio.h>
#include <http_server/stream/convert_bufseq.h>

#include <boost/function.hpp>
#include <vector>

namespace http {
namespace stream {

template <
    typename Handler                = boost::function<void (
                                          sys::error_code const&, std::size_t)>
  , typename MutableBuffer          = asio::mutable_buffer
  , typename ConstBuffer            = asio::const_buffer
  , typename MutableBufferSequence  = std::vector<MutableBuffer>
  , typename ConstBufferSequence    = std::vector<ConstBufferSequence>
>
class basic 
{
public:
  typedef Handler               handler_type;
  typedef MutableBuffer         mutable_buffer_type;
  typedef ConstBuffer           const_buffer_type;
  typedef MutableBufferSequence mutable_buffer_sequence_type;
  typedef ConstBufferSequence   const_buffer_sequence_type;

public:
	basic (basic* lower_layer) : next_ {} 
	virtual ~basic () {}

	basic* lower_layer () { return next_; }

  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
    void (boost::system::error_code, std::size_t))
	auto async_read_some (MutableBufferSequence const& buffers, 
	    BOOST_ASIO_MOVE_ARG (ReadHandler) handler)
	{
		BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

		return _async_read_some (
		  detail::convert_bufseq<mutable_buffer_sequence_type> (buffers),
		  BOOST_ASIO_MOVE_CAST(ReadHandler)(handler)
		);
  }

  template <typename ConstBufferSequence, typename WriteHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
    void (boost::system::error_code, std::size_t))
	auto async_write_some (ConstBufferSequence const& buffers, 
	    BOOST_ASIO_MOVE_ARG (WrtieHandler) handler)
	{
		BOOST_ASIO_READ_HANDLER_CHECK(WriteHandler, handler) type_check;

		return _async_write_some (
		  detail::convert_bufseq<const_buffer_sequence_type> (buffers),
		  BOOST_ASIO_MOVE_CAST(WriteHandler)(handler)
		);
  }

protected:
  virtual void _async_read_some (
      mutable_buffer_sequence_type const& buffers,
      handler_type handler) = 0;

  virtual std::size_t _async_read_some (
      mutable_buffer_sequence_type const& buffers,
      asio::yield_context yield) = 0;

  virtual void _async_write_some (
      const_buffer_sequence_type const& buffers,
      handler_type handler) = 0;

  virtual std::size_t _async_write_some (
      const_buffer_sequence_type const& buffers,
      asio::yield_context yield) = 0;

private:
  basic* next_;
};

}} // namespace http::stream
#endif // _HTTP_STREAM_BASIC_H_
