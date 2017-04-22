/*This module check the packet USRP has received. Once it confirms correct trasmission, it generates a new ACK with the information from the certain file.
Input:MAC packet
Output:New packet with chip number*/

#include "message_generator_impl.h"
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


int buf;
char text[65]="ABCDEFGHABC";//32
//char text[65]="ABCDEFGHABCDEFGH";
int ct;
int seq_num;
int prev_seq_num;
pmt::pmt_t msg;


using namespace gr::ieee802_15_4;

namespace gr{
	namespace ieee802_15_4{

    message_generator_impl::message_generator_impl()
      : block("message_generator",
              io_signature::make(0, 0, 0),
              io_signature::make(0, 0, 0)),
        d_finished(false)
    {
	  ct = 0;
	  seq_num = 0;
	  prev_seq_num = 0;


      message_port_register_out(pmt::mp("strobe"));
	}

    message_generator_impl::~message_generator_impl(){}

    bool
    message_generator_impl::start()
    {
      // NOTE: d_finished should be something explicitely thread safe. But since
      // nothing breaks on concurrent access, I'll just leave it as bool.
      d_finished = false;
      d_thread = boost::shared_ptr<gr::thread::thread>
        (new gr::thread::thread(boost::bind(&message_generator_impl::run, this)));

	 /* 
	  FILE *f = fopen("/home/captain/test/test_file","r");
	  char t;
	  while((t = fgetc(f)) != EOF){
		text[ct++] = t;
	  }
	  text[ct] = '\0';
	  msg = pmt::make_blob(text,strlen(text));
	  */
	  //std::cout << "!!!!!!" << std::endl;
      return block::start();
    }

    bool
    message_generator_impl::stop()
    {
      // Shut down the thread
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();

      return block::stop();
    }

    void message_generator_impl::run()
    {
	  //int t = 10;
	  unsigned char *tmp;
				
		msg = pmt::make_blob(text,strlen(text));
		message_port_pub(pmt::mp("strobe"),msg);
		tmp = (unsigned char*)pmt::blob_data(msg);
		/*
			for(int i = 0; i < pmt::blob_length(msg);i++){
				std::cout << int(tmp[i]) <<" ";
			}std::cout << std::endl;
			*/
    }

message_generator::sptr
message_generator::make(){
	return gnuradio::get_initial_sptr
		(new message_generator_impl());
}

}/*namespace ieee802.15.4 */
}/*namespace gr */

