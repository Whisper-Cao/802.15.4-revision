/*This module check the packet USRP has received. Once it confirms correct trasmission, it generates a new ACK with the information from the certain file.
Input:MAC packet
Output:New packet with chip number*/

#include "ieee802_15_4/message_feedback.h"
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

#define dout std::cout

using namespace gr::ieee802_15_4;

class message_feedback_impl : public message_feedback{

public:

	message_feedback_impl()
		:block("message_feedback",
				gr::io_signature::make(0,0,0),
				gr::io_signature::make(0,0,0))
	{	

		message_port_register_in(pmt::mp("from pdu in"));
		set_msg_handler(pmt::mp("from pdu in"), boost::bind(&message_feedback_impl::pdu_in,this,_1));
		message_port_register_out(pmt::mp("out to RS"));
	}

	~message_feedback_impl(void){
	}

	void pdu_in(pmt::pmt_t msg){
		pmt::pmt_t blob;
		if(pmt::is_pair(msg)){
			blob = pmt::cdr(msg);
		}else{
			assert(false);
		}

		size_t data_len = pmt::blob_length(blob);
		

		if(data_len < 11){
			dout << "No feedback because of short frame" << std::endl;
			return;
		}

		uint16_t crc = crc16((char*)pmt::blob_data(blob),data_len);
		if(!crc){
			message_feedback_impl::fb_chip_num();
		}
	}

	void fb_chip_num(){
		FILE *f = fopen("/home/captain/test/receiver/ack","r");
		int chip_num;
		if(f != NULL)
		//	fprintf(stderr,"Here\n");
			fscanf(f,"%d",&chip_num);
	//	fprintf(stderr,"!!!!!!!\n");
	//	int x;
	//	x = fscanf(f,"%d");
//		fprintf(stderr,"Chip number is %d\n",x);
//		chip_num = &x;
		fclose(f);
		std::memcpy(&buf,&chip_num,sizeof(int));
		char buff[] = "Test Message!";
		pmt::pmt_t msg = pmt::make_blob(buff,50);
		message_port_pub(pmt::mp("out to RS"),msg);
		fprintf(stderr,"we have give a feedback of %d\n",chip_num);
	}

	uint16_t crc16(char *buf, int len){
		uint16_t crc = 0;

		for(int i = 0; i < len; i++){
			for(int k = 0; k < 8; k++){
				int input_bit = (!!(buf[i]& (1 << k)) ^ (crc & 1));
				crc = crc >> 1;
				if(input_bit){
					crc ^= (1 << 15);
					crc ^= (1 << 10);
					crc ^= (1 << 3);
				}
			}
		}
		return crc;
	}

private:
	int buf;
};

message_feedback::sptr
message_feedback::make(){
	return gnuradio::get_initial_sptr
		(new message_feedback_impl());
}

