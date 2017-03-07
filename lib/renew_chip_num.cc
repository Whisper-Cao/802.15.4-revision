#include <ieee802_15_4/renew_chip_num.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>

using namespace gr::ieee802_15_4;

class renew_chip_num_impl : public renew_chip_num{
	public:
		renew_chip_num_impl():
			block("renew_chip_num",
					gr::io_signature::make(0,0,0),
					gr::io_signature::make(0,0,0))
		{
			pmt::pmt_t mac_in = pmt::mp("ack in");
			message_port_register_in(mac_in);
			set_msg_handler(mac_in,
						boost::bind(&renew_chip_num_impl::renew_work,this,_1));
		}

		~renew_chip_num_impl(void){

		};
		
		void renew_work(pmt::pmt_t msg){
			pmt::pmt_t blob;
			if(pmt::is_pair(msg)){
				blob = pmt::cdr(msg);
			}else{
				assert(false);
			}
			unsigned char buf[256];
			size_t data_len = pmt::blob_length(blob);
			std::memcpy(buf,(char*)pmt::blob_data(blob),data_len);
/*
			for(int i = 0; i < data_len;i++){
				std::cout << int(buf[i]) << " ";
			}std::cout << std::endl;
*/

			if(data_len >= 5 && int(buf[0] == 129) && (buf[1] == 0) &&
					buf[2] == 23 && buf[3] == 42){
				FILE *f = fopen("/home/captain/test/transceiver/rec_ack","w+");
				time_t tt = time(NULL);
				tm *t = localtime(&tt);
				fprintf(f,"%c\ntimestamp:%d-%02d-%02d %02d:%02d:%02d\n",
						buf[4],t->tm_year+1900,t->tm_mon+1,t->tm_mday,
						t->tm_hour,t->tm_min,t->tm_sec);
				fclose(f);
			}
		}
				
};

renew_chip_num::sptr
renew_chip_num::make(){
	return gnuradio::get_initial_sptr
		(new renew_chip_num_impl());
}
