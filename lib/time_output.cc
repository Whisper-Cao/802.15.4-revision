#include <ieee802_15_4/time_output.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace gr::ieee802_15_4;


class time_output_impl : public time_output {
public:
	int flag;

	time_output_impl() :
		block ("time_output",
			gr::io_signature::make(1, -1, sizeof(gr_complex)),
			gr::io_signature::make(0, 0, 0))
	{
			struct timeval a;
			gettimeofday(&a,NULL);
			fprintf(stderr,"Second Sending time: %lld\n",a.tv_usec);

	//	message_port_register_in(pmt::mp("msg"));
	//	set_msg_handler(pmt::mp("msg"), boost::bind(&time_output_impl::work, this, _1));
		flag = 0;
	}

	~time_output_impl(void) {
	}

	int general_work(int noutput, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items){
		//	if(flag == 0){
		//	std::cout << noutput << std::endl;
			struct timeval a;
			gettimeofday(&a,NULL);
			fprintf(stderr,"Sending time: %lld\n",a.tv_usec);
		//	flag = 1;}
			return 0;
	}


};


time_output::sptr
time_output::make() {
	return gnuradio::get_initial_sptr(new time_output_impl());
}
