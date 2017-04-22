/*This module check the packet USRP has received. Once it confirms correct trasmission, it generates a new ACK with the information from the certain file.
Input:MAC packet
Output:New packet with chip number*/

#include "feedback_generator_impl.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <gnuradio/block_detail.h>

char buf_fb[10];
pmt::pmt_t msg_fb;

using namespace gr::ieee802_15_4;

namespace gr{
	namespace ieee802_15_4{

    feedback_generator_impl::feedback_generator_impl()
      : block("feedback_generator",
              io_signature::make(0, 0, 0),
              io_signature::make(0, 0, 0)),
        d_finished(false)
    {;
      message_port_register_out(pmt::mp("out to RS"));
	}

    feedback_generator_impl::~feedback_generator_impl(){}

    bool
    feedback_generator_impl::start()
    {
      // NOTE: d_finished should be something explicitely thread safe. But since
      // nothing breaks on concurrent access, I'll just leave it as bool.
      d_finished = false;
      d_thread = boost::shared_ptr<gr::thread::thread>
        (new gr::thread::thread(boost::bind(&feedback_generator_impl::run, this)));
	  FILE *f = fopen("/home/captain/test/receiver/ack","r");
	  int chip_num;

	  if(f != NULL)
		fscanf(f,"%d",&chip_num);
	  fclose(f);
	  std::memcpy(&buf_fb,&chip_num,sizeof(int));
	
	  msg_fb = pmt::make_blob(buf_fb,strlen(buf_fb));
	//  message_port_pub(pmt::mp("out to RS"),msg);
	  fprintf(stderr,"we have give a feedback of %d\n",chip_num);
      message_port_pub(pmt::mp("out to RS"), msg_fb);

	  
      return block::start();
    }

    bool
    feedback_generator_impl::stop()
    {
      // Shut down the thread
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();

      return block::stop();
    }

    void feedback_generator_impl::run()
    {
       // message_port_pub(pmt::mp("out to RS"), msg_fb);
    }

	void fb_chip_num(){
	}

feedback_generator::sptr
feedback_generator::make(){
	return gnuradio::get_initial_sptr
		(new feedback_generator_impl());
}

}/*namespace ieee802.15.4 */
}/*namespace gr */

