#ifndef INCLUDED_IEEE802_15_4_FEEDBACK_GENERATOR_IMPL_H
#define INCLUDED_IEEE802_15_4_FEEDBACK_GENERATOR_IMPL_H

#include <ieee802_15_4/feedback_generator.h>

namespace gr {
  namespace ieee802_15_4 {
	class feedback_generator_impl : public feedback_generator
	{
	private:
	  boost::shared_ptr<gr::thread::thread> d_thread;
	  bool d_finished;

	  void run();

	public:
	  feedback_generator_impl();
	  ~feedback_generator_impl();


	  // Overloading these to start and stop the internal thread that
	  // periodically produces the message.
	  bool start();
	  bool stop();
	};
}
}

#endif
