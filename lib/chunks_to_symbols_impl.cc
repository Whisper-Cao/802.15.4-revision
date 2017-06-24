#include "chunks_to_symbols_impl.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <gnuradio/tag_checker.h>
#include <iostream>
#include <fstream>
#include <gnuradio/basic_block.h>
#include <gnuradio/config.h>

using namespace gr::ieee802_15_4;

std::vector<gr_complex> d64_symbol_table;
std::vector<gr_complex> d32_symbol_table;
std::vector<gr_complex> d16_symbol_table;
std::vector<gr_complex> d4_symbol_table;

std::vector<gr_complex> header_symbol_table;

const int LSB_TO_MSB[16]={
	0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15
};

const int table_64[] = {
-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,
-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,
-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,
-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,
-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,
-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,
1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,
1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,
-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,
-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,
1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,
1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,
-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,
-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,
-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,-1,
-1,1,1,1,-1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,1
};

const int table_32[] = {
1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,
1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,
-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,
-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,
-1,-1,1,-1,1,1,1,-1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,
-1,1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,
1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,1,1,-1,1,1,-1,-1,1,
1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,
1,1,1,-1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,
1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,
-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,
-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,
-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,1,1,-1,1,1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,
-1,1,1,1,-1,1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,
1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,1,1,1,-1,1,1,-1,1,
1,1,-1,-1,1,-1,-1,1,-1,1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,1,1,-1,-1,-1
};

const int table_16[] = {
-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,
-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,
-1,-1,1,-1,-1,1,-1,1,-1,-1,1,1,1,1,1,-1,
-1,1,1,1,-1,-1,-1,-1,-1,1,1,-1,1,-1,1,1,
-1,1,-1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,
-1,-1,-1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,
1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,-1,1,1,
1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,1,1,-1,
-1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,1,
-1,-1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1,-1,
1,-1,-1,-1,1,-1,-1,1,-1,1,-1,-1,1,1,1,1,
1,1,-1,1,1,1,-1,-1,-1,-1,-1,1,1,-1,1,-1,
1,-1,-1,1,-1,1,-1,-1,1,1,1,1,1,-1,-1,-1,
1,1,-1,-1,-1,-1,-1,1,1,-1,1,-1,1,1,-1,1,
1,1,1,1,1,-1,-1,-1,1,-1,-1,1,-1,1,-1,-1,
1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,1
};

const int table_4[] = {
-1,-1,-1,-1,-1,-1,
-1,1,-1,-1,-1,-1,
-1,-1,1,-1,-1,-1,
-1,1,1,-1,-1,-1,
-1,-1,-1,1,-1,-1,
-1,1,-1,1,-1,-1,
-1,-1,1,1,-1,-1,
-1,1,1,1,-1,-1,

-1,-1,-1,-1,1,-1,
-1,1,-1,-1,1,-1,
-1,-1,1,-1,1,-1,
-1,1,1,-1,1,-1,
-1,-1,-1,1,1,-1,
-1,1,-1,1,1,-1,
-1,-1,1,1,1,-1,
-1,1,1,1,1,-1	

};


int chunks_to_symbols_impl::symbol_table_init(){
	for(int i = 0; i < 512; i++){
		header_symbol_table.push_back(gr_complex(table_64[2*i],table_64[2*i+1]));
	}

		for(int i = 0; i < 512; i++)
			d64_symbol_table.push_back(gr_complex(table_64[2*i],table_64[2*i+1]));
		for(int i = 0; i < 256;i++)
			d32_symbol_table.push_back(gr_complex(table_32[2*i],table_32[2*i+1]));
		for(int i = 0; i < 128;i++)
			d16_symbol_table.push_back(gr_complex(table_16[2*i],table_16[2*i+1]));
		for(int i = 0; i < 48; i++)
			d4_symbol_table.push_back(gr_complex(table_4[2*i],table_4[2*i+1]));
}


int getChipNum(char c){
	if(c == 'A') return 64;
	else if(c == 'B') return 32;
	else if(c == 'C') return 16;
	else if(c == 'D') return 4+2;
	else{
		std::cout << "ReadError: Wrong chip number in the ACK!" << std::endl;
		return 32;
	}
}

chunks_to_symbols::sptr
chunks_to_symbols::make()
{
	return gnuradio::get_initial_sptr(new chunks_to_symbols_impl());
}

chunks_to_symbols_impl::chunks_to_symbols_impl():
	sync_interpolator("chunks_to_symbols",
			io_signature::make(1,-1,sizeof(int8_t)),
			io_signature::make(1,-1,sizeof(gr_complex)),
			32){

//	message_port_register_in(pmt::mp("set_symbol_table"));
//	set_msg_handler(pmt::mp("set_symbol_table"),
//			boost::bind(chunks_to_symbols_impl::symbol_table_init,this,_1));
	}

chunks_to_symbols_impl::~chunks_to_symbols_impl(){

}


int chunks_to_symbols_impl::work(
	int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
    //	this->set_interpolation(16);
	 // fprintf(stderr,"I'm doing chunks to symbols!\n");
	  assert(noutput_items % d_D == 0);
      assert(input_items.size() == output_items.size());
      int nstreams = input_items.size();
	  char tmp[5];
	  FILE *f = fopen("/home/captain/test/transceiver/rec_ack","r");
	  fscanf(f,"%s",tmp);
	  fclose(f);
	  //symbol_table_init(32);
	  int d_D = getChipNum(tmp[0])/2;
	  symbol_table_init();
	  FILE *f2 = fopen("/home/captain/experiment/cer-estimation/send","a+");
	  int ct = 0;
      for(int m = 0 ; m < nstreams; m++) {
        const int8_t *in = (int8_t*)input_items[m];
        gr_complex *out = (gr_complex *)output_items[0];
        std::vector<tag_t> tags;
        get_tags_in_range(tags, m, nitems_read(m), nitems_read(m)+noutput_items/d_D);
        tag_checker tchecker(tags);

        for(int i = 0; i < noutput_items / d_D; i++) {
		  std::vector<tag_t> tags_now;
          tchecker.get_tags(tags_now, i+nitems_read(m));
          for (unsigned int j=0; j<tags_now.size(); j++) {
            tag_t tag = tags_now[j];
            dispatch_msg(tag.key, tag.value);
          }
		  //fprintf(stderr,"%d ",in[i]);
		 // assert(((unsigned int)in[i]*d_D+d_D) <= d_symbol_table.size());
	      if(i < 10){
			memcpy(out,&header_symbol_table[(unsigned int)in[i]*32],32*sizeof(gr_complex));
			out += 32;
		  }
		  else{
			if(ct <= 61) {
				fprintf(f2,"%d ",LSB_TO_MSB[in[i]]);
				ct++;
			}
			if(d_D == 32) memcpy(out, &d64_symbol_table[(unsigned int)in[i]*d_D], d_D*sizeof(gr_complex));
			else if(d_D == 16) memcpy(out, &d32_symbol_table[(unsigned int)in[i]*d_D], d_D*sizeof(gr_complex));
			else if(d_D == 8) memcpy(out, &d16_symbol_table[(unsigned int)in[i]*d_D], d_D*sizeof(gr_complex));
			else if(d_D == 3) memcpy(out, &d4_symbol_table[(unsigned int)in[i]*d_D], d_D*sizeof(gr_complex));
			
			
			out+=d_D;
		  }
		}
		//fprintf(stderr,"!\n");
		//std::cout << std::endl;
      }
	  fprintf(f2,"\n");
	  fclose(f2);
	  struct timeval a;
	  gettimeofday(&a,NULL);
	  fprintf(stderr,"Sending time: %lld\n",a.tv_usec);
	  
      return noutput_items;
}
