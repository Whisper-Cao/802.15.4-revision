#ifndef INCLUDED_IEEE802_15_4_MESSAGE_GENERATOR_IMPL_H
#define INCLUDED_IEEE802_15_4_MESSAGE_GENERATOR_IMPL_H

#include <ieee802_15_4/message_generator.h>

namespace gr {
  namespace ieee802_15_4 {
	class message_generator_impl : public message_generator
	{
	private:
	  boost::shared_ptr<gr::thread::thread> d_thread;
	  bool d_finished;

	  void run();

	public:
	  message_generator_impl();
	  ~message_generator_impl();


	  // Overloading these to start and stop the internal thread that
	  // periodically produces the message.
	  bool start();
	  bool stop();
	};
}
}

#endif
