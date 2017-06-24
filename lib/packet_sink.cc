/*
 * Copyright 2004,2013 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ieee802_15_4/packet_sink.h>
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <gnuradio/blocks/count_bits.h>
#include <iostream>
#include <math.h>

using namespace gr::ieee802_15_4;

// very verbose output for almost each sample
#define VERBOSE 0
// less verbose output for higher level debugging
#define VERBOSE2 0
#define VERBOSE3 1
// this is the mapping between chips and symbols if we do
// a fm demodulation of the O-QPSK signal. Note that this
// is different than the O-QPSK chip sequence from the
// 802.15.4 standard since there there is a translation
// happening.
// See "CMOS RFIC Architectures for IEEE 802.15.4 Networks",
// John Notor, Anthony Caviglia, Gary Levy, for more details.
static const unsigned int CHIP_MAPPING[] = {
					1618456172,
					1309113062,
					1826650030,
					1724778362,
					778887287,
					2061946375,
					2007919840,
					125494990,
					529027475,
					838370585,
					320833617,
					422705285,
					1368596360,
					85537272,
					139563807,
					2021988657};
//revision begins
static const unsigned int CHIP_MAPPING_2[] = {
					29794,
					15640,
					12102,
					3025,
					25332,
					6333,
					17967,
					20875,
					2973,
					17127,
					20665,
					29742,
					7435,
					26434,
					14800,
					11892};
static const unsigned int CHIP_MAPPING_3[] = {
	21,22,19,16,25,26,31,28,13,14,11,8,1,2,7,4
};
static const long long unsigned int CHIP_MAPPING_4[] = {
8386371523260976029,
2122429490862648331,
836953071634853484,
7785489504032290734,
3345061394119811191,
8623949133603168864,
6951216328944219234,
7088797404501876084,
837000513593799778,
7100942545992127476,
8386418965219922323,
1437882532822485073,
5878310642734964616,
599422903251606943,
2272155707910556573,
2134574632352899723
};
//revision ends
static const int MAX_PKT_LEN    = 128 -  1; // remove header and CRC
static const int MAX_LQI_SAMPLES = 8; // Number of chip correlation samples to take

class packet_sink_impl : public packet_sink {
public:

void enter_search()
{
	if (VERBOSE)
		fprintf(stderr, "@ enter_search\n");

	d_state = STATE_SYNC_SEARCH;
	d_shift_reg = 0;
	d_preamble_cnt = 0;
	d_chip_cnt = 0;
	d_packet_byte = 0;
}
    
void enter_have_sync()
{
	if (VERBOSE)
		fprintf(stderr, "@ enter_have_sync\n");

	d_state = STATE_HAVE_SYNC;
	d_packetlen_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;

	// Link Quality Information
	d_lqi = 0;
	d_lqi_sample_count = 0;
}

void enter_have_header(int payload_len)
{
	if (VERBOSE)
		fprintf(stderr, "@ enter_have_header (payload_len = %d)\n", payload_len);

	d_state = STATE_HAVE_HEADER;
	d_packetlen  = payload_len;
	d_payload_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;
	for(int i = 0; i < 4; i++){
		be1[i] = 0;
		be2[i] = 0;
	}
}



unsigned char decode_chips(unsigned long long int chips){
	int i;
	int best_match = 0xFF;
	int min_threshold = d_chip_num + 1; // Matching to 32 or 16 chips, could never have a error of 33 chips
	bool notSync = (d_state == STATE_HAVE_HEADER);
	bool flag = (have_header == 1) && VERBOSE3;


	for(i=0; i<16; i++) {
		// FIXME: we can store the last chip
		// ignore the first and last chip since it depends on the last chip.
		//revision begins
		unsigned int threshold;
		if(d_chip_num == 32)
			threshold = gr::blocks::count_bits32((chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[i] & 0x7FFFFFFE));
		else if(d_chip_num == 16)
			threshold = gr::blocks::count_bits16((chips & 0x7FFE) ^( CHIP_MAPPING_2[i] & 0x7FFE));
		else if(d_chip_num == 4)
			threshold = gr::blocks::count_bits32((chips & 0x1E) ^( CHIP_MAPPING_3[i] & 0x1E));
		else
			threshold = gr::blocks::count_bits64((chips & 0x7FFFFFFFFFFFFFFE) ^( CHIP_MAPPING_4[i] & 0x7FFFFFFFFFFFFFFE));
		//revision ends
		if (threshold < min_threshold) {
			best_match = i;
			min_threshold = threshold;
		}
	}
	unsigned long long x_chips= chips;
	
	if(d_chip_num == 32){
		x_chips = chips & 0xFFFFFFFF;
	}
	else if(d_chip_num == 16){
		x_chips = chips &0xFFFF;
	}
	else if(d_chip_num == 4){
		x_chips = chips & 0x1E;
	}
	received = fopen("/home/captain/experiment/snr/received","ab");

	fprintf(received,"%llx  ",x_chips);
	fclose(received);
//	fprintf(sequence,"%d ",best_match);
//	fprintf(estimated," ");
//	flag = true;
	flag = false;
	if (min_threshold < d_threshold) {
		//error_count += count_error(chips,d_chip_num,best_match);
		if (flag && d_chip_num == 32){
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[best_match] & 0x7FFFFFFE)), fflush(stderr);
		// LQI: Average number of chips correct * MAX_LQI_SAMPLES
		}else if(flag && d_chip_num == 16){
				fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x7FFE) ^ (CHIP_MAPPING_2[best_match] & 0x7FFE)), fflush(stderr);
		}else if(flag && d_chip_num == 4){
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x1E) ^ (CHIP_MAPPING_3[best_match] & 0x1E)), fflush(stderr);			
		}else if(flag && d_chip_num == 64){
			fprintf(stderr, "Found sequence with %d errors at 0x%x, send: %x,receive:%x\n", min_threshold, (chips & 0x7FFFFFFFFFFFFFF0) ^ (CHIP_MAPPING_4[best_match]), fflush(stderr));
		}
		if (d_lqi_sample_count < MAX_LQI_SAMPLES) {
			d_lqi += d_chip_num - min_threshold;//It is 32 originally
			d_lqi_sample_count++;
		}
	//	fprintf(received,"%llx ",chips);
		return (char)best_match & 0xF;
	}
	else{
		//error_count += d_chip_num;
	//	fprintf(received,"%llx ",chips);
	}

	return 0xFF;
}

unsigned int count_error(long long int chips,int chip_num, int match){
	//fprintf(stderr,"match is: %d\n",(chips & 0x1F));
	if(match < 16){
		if(chip_num == 64)
			return gr::blocks::count_bits64((chips & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[match] & 0x7FFFFFFFFFFFFFFE));
		else if(chip_num == 32)
			return gr::blocks::count_bits32((chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[match] & 0x7FFFFFFE));
		else if(chip_num == 16)	
			return gr::blocks::count_bits16((chips & 0x7FFE) ^ (CHIP_MAPPING_2[match] & 0x7FFE));
		else if(chip_num == 4)	
			return gr::blocks::count_bits8((chips & 0x1E) ^ (CHIP_MAPPING_3[match] & 0x1E));
	}else{
		return d_threshold;
	}
}

int slice(float x){
	return x>0?1:0;
}

void compute_chip(double x,int index){
	double d1 = fabs(x-1);
	double d0 = fabs(x+1);
	per_chip_0_pos[index] = d1/(d0+d1);
}

void compute_per_bit(unsigned char x){
	unsigned long long int table[16];
	if(d_chip_num == 4){
		for(int i = 0; i< 16;i++) table[i] = CHIP_MAPPING_3[i];
	}
	else if(d_chip_num == 16){
		for(int i = 0; i< 16;i++) table[i] = CHIP_MAPPING_2[i];
	}
	else if(d_chip_num == 32){
		for(int i = 0; i< 16;i++) table[i] = CHIP_MAPPING[i];
	}
	else if(d_chip_num == 64){
		for(int i = 0; i< 16;i++) table[i] = CHIP_MAPPING_4[i];
	}
/*	
	for(int j = 0; j < d_chip_num; j++){
		fprintf(stderr,"%f ",per_chip_0_pos[j]);
	}fprintf(stderr,"\n");
	fprintf(stderr,"%d\n",x);
	fprintf(stderr,"%x\n",d_shift_reg);
*/
	unsigned long long int sym = 0;
	unsigned long long int c = 1;
	double pos = 1;
	int dd = d_chip_num;
	if(dd == 4) dd = 6;
	for(int i = 0; i < 16;i++){
		sym = table[i];
		pos = 1;
		sym = sym >> 1;		
		for(int j = 1; j < dd-1;j++){
			if(sym & 1 == 1){
				pos *=8* (1-per_chip_0_pos[dd-j-1]);
			}
			else{
				pos *= 8 * per_chip_0_pos[dd-j-1];
			}
			sym = sym >> 1;
		}
		per_bit_pos[i%16] = pos;
	}
}

double compute_be(int symbol,unsigned long long int chips){
	double be = 0;
	double p = 0;
	double tmpmx = 0;
	for(int i = 0; i < 16; i++){
		p += per_bit_pos[i];
	}
	FILE *f_prob = fopen("/home/captain/experiment/ber-estimation/probability","ab");
	for(int i = 0; i < 16; i++){
		per_bit_pos[i] = per_bit_pos[i] / p;
		fprintf(f_prob,"%f ",per_bit_pos[i]);
	}
	fclose(f_prob);
	int chiperrors = getChipDifferences(chips);
	if(symbol <= 15){
		be = 2*ber2_impact_adj((1-per_bit_pos[symbol]))*w2;
		//`be = 4*(1-per_bit_pos[symbol])*w2;
		be2[map(d_chip_num)] += be;
		estimate_ber_for_remainings(1,chiperrors,be);
		/*
		for(int i = 0;i<16; i++){
			if(per_bit_pos[i] > tmpmx){
				tmpmx = per_bit_pos[i];
			}
				//be += 4*per_bit_pos[i]*count_error(chips,d_chip_num,i)/d_threshold;
			
		}
		be = 4*(1-tmpmx);*/
	}
	else {
		be = 2*w1;
		be1[map(d_chip_num)] += be;
		estimate_ber_for_remainings(0,chiperrors,be);
	}
	//fprintf(stderr,"~~~%f\n",be2[map(d_chip_num)]);
	return be;
}

double ber2_impact_adj(double x){
	
	if(x < 0.2) return x;
	else if(x >= 0.2 && x < 0.25) return 0.05;
	else if(x >= 0.25 && x < 0.3) return 0.3;
	else if(x >= 0.3 && x < 0.4) return 0.5;
	//else if(x >= 0.4 && x < 0.5) return 0.35;
	else if(x >= 0.4 && x < 1) return 0.8;
	else return x;
	return x;
}

int getChipDifferences(unsigned long long int chips){
	int min_threshold = 100;
	int best_match = 0;
	for(int i=0; i<16; i++) {
		// FIXME: we can store the last chip
		// ignore the first and last chip since it depends on the last chip.
		//revision begins
		unsigned int threshold;
		if(d_chip_num == 32)
			threshold = gr::blocks::count_bits32((chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[i] & 0x7FFFFFFE));
		else if(d_chip_num == 16)
			threshold = gr::blocks::count_bits16((chips & 0x7FFE) ^( CHIP_MAPPING_2[i] & 0x7FFE));
		else if(d_chip_num == 4)
			threshold = gr::blocks::count_bits32((chips & 0x1E) ^( CHIP_MAPPING_3[i] & 0x1E));
		else
			threshold = gr::blocks::count_bits64((chips & 0x7FFFFFFFFFFFFFFE) ^( CHIP_MAPPING_4[i] & 0x7FFFFFFFFFFFFFFE));
		//revision ends
		if (threshold < min_threshold) {
			best_match = i;
			min_threshold = threshold;
		}
	}
	return min_threshold;
}

void SelectThreshold(int a){
	//d_threshold = est_threshold[map(d_chip_num)];
}

void SelectRate(){
	double be[4];
	int index = map(d_chip_num);
	double mybe = be1[index]+be2[index];

	for(int i = 0; i < 4; i++){
		be[i] = be1[i] + be2[i];
		//fprintf(stderr,"%f  %f  \n",be1[i],be2[i]);
	}
	//fprintf(stderr,"be is:%f \n",mybe);
	fprintf(stderr,"Bit error rate: %.3f\n",(be1[index]+be2[index]/4)/256);
	char feedback;
	double th[4][3] = {{-1,-1,0.000001},{-1,0.000005,0.04},{0.5,20,50},{160,400,800}};
	if(mybe < th[index][0]) 
		feedback = 'D';
	else if(mybe >= th[index][0] && mybe <= th[index][1]) 
		feedback = 'C';
	else if(mybe >= th[index][1] && mybe <= th[index][2]) 
		feedback = 'B';
	else feedback = 'A';
	int char_to_length[4] = {64,32,16,8};
	int s = (d_chip_num == 4) ? 8:d_chip_num;
	fprintf(stderr,"The spreading length is: %d\n",s);
	FILE *tmpf = fopen("/home/captain/test/transceiver/rec_ack","w+");
	fprintf(tmpf,"%c",feedback);
	fclose(tmpf);
}

int map(int chip_num){
	if(chip_num == 64)return 0;
	else if(chip_num == 32) return 1;
	else if(chip_num == 16) return 2;
	else if(chip_num == 4) return 3;
	else return -1;
}

int estimate_ber_for_remainings(int error_type, int chiperrors,double be2_pos){//error_type, 0 is ber1, 1 is ber2
	double tmp;
	int chip[4] = {64,32,16,4};
	//fprintf(stderr,"%d,%d,%f\n",error_type,chip_errors,be2_pos);
	if(error_type == 1){	
		for(int i = 0; i < 4; i++){
			if(chip[i] != d_chip_num){
				tmp = double(chip[i])/d_chip_num * chiperrors;
				if(tmp > est_threshold[i]){
					be1[i] += 4*w1;
				}
				else{
					be2[i] += be2_pos;
				}
			}
		}
	}
	else{
		for(int i = 0; i < 4; i++){
			if(chip[i] != d_chip_num){
				tmp = double(chip[i])/d_chip_num * chiperrors;
				if(tmp > est_threshold[i]){
					be1[i] += 4*w1;
				}
				else{
					be2[i] += 0;
				}
			}
		}
	}
	return 0;
}

void SoftRate(){
	int r[4] = {64,32,16,4};
	int index = map(d_chip_num);
	double th[4][3] = {{-1,-1,9},{-1,7,12},{2,5,9},{270,400,800}};

	unsigned char feedback;
	double mybe;
	if(d_chip_num != 4){
		mybe = chip_errors/d_threshold;
	}
	else{
		mybe = be2[index];
	}
	fprintf(stderr,"%f\n",mybe);
	if(mybe < th[index][0]) 
		feedback = 'D';
	else if(mybe >= th[index][0] && mybe <= th[index][1]) 
		feedback = 'C';
	else if(mybe >= th[index][1] && mybe <= th[index][2]) 
		feedback = 'B';
	else feedback = 'A';
	FILE *tmpf = fopen("/home/captain/test/transceiver/rec_ack","w+");
	fprintf(tmpf,"%c",feedback);
	fclose(tmpf);

}

void RateAdaptation(int type){//0:MrZ  1:softrate 2:traditional zigbee
	if(type == 0){
		SelectRate();
	}
	else if(type == 1){
		SoftRate();
	}
	else{
		//do nothing
	}
}

void receive_frame(){
	fprintf(stderr,"Spreading Length is: %d\n",d_chip_num);
	int type = 2;
	SaveResult(type);
//	estimated = fopen("/home/captain/experiment/cer-estimation/estimated","ab");	
	received = fopen("/home/captain/experiment/snr/received","ab");
	fprintf(received,"\n");
	fclose(received);
	FILE *body_error = fopen("/home/captain/experiment/cer-estimation/bodyerror","ab");
	fprintf(body_error,"%d\n",chip_errors);
	fclose(body_error);
	SelectThreshold(chip_errors);
	FILE *dump = fopen("/home/captain/experiment/selection/dumped","ab");
	fprintf(dump,"%d\n",dumped);
	fclose(dump);
	//SelectRate();	
	RateAdaptation(type);
	chip_errors = 0;

	sym_cnt = 0;
	
	 FILE *be_estimated = fopen("/home/captain/experiment/ber-estimation/estimated","ab");
	 for(int i = 0; i < bct;i++){
	 fprintf(be_estimated,"%f ",be[i]);
	 }
	 fprintf(be_estimated,"\n");
	 fclose(be_estimated);
	received_be = fopen("/home/captain/experiment/ber-estimation/received_be","ab");
	fprintf(received_be,"\n");
	fclose(received_be);

	FILE *f_prob = fopen("/home/captain/experiment/ber-estimation/probability","ab");
	fprintf(f_prob,"\n");
	fclose(f_prob);

	bct = 0;
	dumped = 0;
	//d_shift_reg = 0;
}

void receive_symbol(unsigned long long int d_shift_reg,unsigned char c){
	if(sym_cnt != 4 && sym_cnt != 5){
		int tmpcnt = count_error(d_shift_reg,d_chip_num,c);
		chip_errors += tmpcnt;
	}
	sym_cnt++;
	compute_per_bit(c);
	be[bct] = compute_be(c,d_shift_reg);
	bct++;
	received_be = fopen("/home/captain/experiment/ber-estimation/received_be","ab");
	fprintf(received_be,"%d ",c);
	fclose(received_be);

}

void SaveResult(int type){
	if(type == 0) {
		FILE *res = fopen("/home/captain/experiment/throughput/static-ec/mrz","ab");
		fprintf(res,"%d\t",d_chip_num);
		fclose(res);
	}
	else if(type == 1){
		FILE *res = fopen("/home/captain/experiment/throughput/static-ec/softrate","ab");
		fprintf(res,"%d\t",d_chip_num);
		fclose(res);
	}
	else{
		FILE *res = fopen("/home/captain/experiment/throughput/static-ec/zigbee","ab");
		fprintf(res,"%d\t",d_chip_num);
		fclose(res);
	}
}

packet_sink_impl(int threshold,int chip_num)
  : block ("packet_sink",
		   gr::io_signature::make(1, 1, sizeof(float)),
		   gr::io_signature::make(0, 0, sizeof(int8_t))),
    d_threshold(threshold)
{
	d_sync_vector = 0xA7;
	//d_chip_num = chip_num;
	// Link Quality Information
	d_lqi = 0;
	d_lqi_sample_count = 0;

	d_head_threshold = 17;
	ce_preamble = 0;
	//d_threshold = 5;
	if ( VERBOSE )
		fprintf(stderr, "syncvec: %x, threshold: %d\n", d_sync_vector, d_threshold),fflush(stderr);
	enter_search();

	bct = 0;
	chip_errors = 0;
	sym_cnt = 0;
	w1=1;
	w2=4;
	ber_cost_upperbound = 20;
	est_threshold[0] = 18;
	est_threshold[1] = 8;
	est_threshold[2] = 4;
	est_threshold[3] = 1;
	dumped = 0;
	message_port_register_out(pmt::mp("out"));
	

}

~packet_sink_impl()
{
}

int general_work(int noutput, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items ) {
	const float *inbuf = (const float*)input_items[0];
    int ninput = ninput_items[0];
	int count=0;
	int i = 0;
	
	unsigned char new_chip_num = 0;
	int ce_frame = 0;
	int tmp_ce = 0;
	int isFirst = 0;
	
	if (VERBOSE)
		fprintf(stderr,">>> Entering state machine\n"),fflush(stderr);
	while(count < ninput) {
		switch(d_state) {

		case STATE_SYNC_SEARCH:    // Look for sync vector
		//	have_header = 0;
			isFirst = 0;
			if (VERBOSE)
				fprintf(stderr,"SYNC Search, ninput=%d syncvec=%x\n", ninput, d_sync_vector),fflush(stderr);

			while (count < ninput) {
				
				if(slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				if(d_preamble_cnt > 0){
					d_chip_cnt = d_chip_cnt+1;
				}
					// The first if block syncronizes to chip sequences.
					if(d_preamble_cnt == 0){
						unsigned int threshold;
						threshold = gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[0] & 0x7FFFFFFFFFFFFFFE));
						if(threshold < d_head_threshold) {
							ce_preamble = 0;
							ce_preamble += threshold;
							if (VERBOSE2)
								fprintf(stderr,"Found 0 in chip sequence\n"),fflush(stderr);
							// we found a 0 in the chip sequence
							d_preamble_cnt+=1;
							//fprintf(stderr, "Threshold %d d_preamble_cnt: %d\n", threshold, d_preamble_cnt);						
						}
					} else {
						// we found the first 0, thus we only have to do the calculation every 32 chips
						if(d_chip_cnt == 64){
							d_chip_cnt = 0;
							//unsigned char c = decode_chips(d_shift_reg);
							//fprintf(stderr,"%d\n",c);
							if(d_packet_byte == 0) {
								if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[0] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold) {
									ce_preamble += gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[0] & 0xFFFFFFFFFFFFFFFE));
									if (VERBOSE2)
										fprintf(stderr,"Found %d 0 in chip sequence\n", d_preamble_cnt),fflush(stderr);
									// we found an other 0 in the chip sequence
									d_packet_byte = 0;
									d_preamble_cnt ++;
								}else if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[7] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold) {
									ce_preamble += gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[7] & 0xFFFFFFFFFFFFFFFE));
									if (VERBOSE2)
										fprintf(stderr,"Found first SFD\n"),fflush(stderr);
									d_packet_byte = 7 << 4;
								}else {
									// we are not in the synchronization header
									if (VERBOSE2)
										fprintf(stderr, "Wrong first byte of SFD. %u\n", d_shift_reg), fflush(stderr);
									enter_search();
								
									break;
								}
							} else {
								//fprintf(f,"%d\n",ce_preamble);
								//fprintf(f,"%d\n",ce_preamble);
							
								//fclose(f);

								if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[10] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold
										) {
									ce_preamble += gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[10] & 0xFFFFFFFFFFFFFFFE));	
								//	fprintf(f,"%d\n",ce_preamble);
								//	fclose(f);
									d_packet_byte |= 0xA;
									d_chip_num = 64;
									d_threshold = 23;
									if (VERBOSE2)
										fprintf(stderr,"Found sync, 0x%x\n", d_packet_byte),fflush(stderr);
									// found SDF
									// setup for header decode
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[11] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold){
							//		fprintf(f,"%d\n",ce_preamble);
									
									ce_preamble += gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[11] & 0xFFFFFFFFFFFFFFFE));
							//		fprintf(f,"%d\n",ce_preamble);
								//	fclose(f);
									d_packet_byte |= 0xB;
									d_chip_num = 32;
									d_threshold = 10;
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[12] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold){
									ce_preamble += 	gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[12] & 0xFFFFFFFFFFFFFFFE));
									d_packet_byte |= 0xC;
									d_chip_num = 16;
									d_threshold = 4;
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[13] & 0xFFFFFFFFFFFFFFFE)) <= d_head_threshold){
									ce_preamble +=  gr::blocks::count_bits64((d_shift_reg & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[13] & 0xFFFFFFFFFFFFFFFE));
									d_packet_byte |= 0xD;
									d_chip_num = 4;
									d_threshold = 1;
									enter_have_sync();
									break;
								}
								 else {
									if (VERBOSE)
										fprintf(stderr, "Wrong second byte of SFD. %u\n", d_shift_reg), fflush(stderr);
								//	enter_search();
								//	break;
								}
								 //fprintf(stderr,"!\n");
							}
						}
					
					}//d_premble_cnt == 0 ends
				//my revision
			}//while ends
			break;

		case STATE_HAVE_SYNC:
			have_header = 0;
			//fprintf(stderr,"%d\n",ce_preamble);
			//reset_ber();
			if (VERBOSE2)
				fprintf(stderr,"Header Search bitcnt=%d, header=0x%08x\n", d_headerbitlen_cnt, d_header),
				fflush(stderr);

			while (count < ninput) {		// Decode the bytes one after another.	
				if(slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				d_chip_cnt = d_chip_cnt+1;
				
				if(d_chip_num == 32){
					if(d_chip_cnt == 32){
						d_chip_cnt = 0;
						unsigned char c = decode_chips(d_shift_reg);
						if(c == 0xFF){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							//enter_search();
							//break;
						}

						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte which represents the frame length.
							int frame_len = d_packet_byte;
							if(frame_len <= MAX_PKT_LEN){
								enter_have_header(frame_len);
							} else {
								enter_have_header(32);
								//enter_search();
							}
							break;
						}
					}
				}
				//my revision
				else if(d_chip_num == 16){
					if(d_chip_cnt == 16){
						d_chip_cnt = 0;
						unsigned char c = decode_chips(d_shift_reg);
						//fprintf(stream2,"%d\n",c);

						if(c == 0xFF){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);
				//			enter_search();
				//			break;
						}

						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							d_packet_byte |= c << 4;
						}
						d_packet_byte = 32;
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte which represents the frame length.
							int frame_len = d_packet_byte;
							if(frame_len <= MAX_PKT_LEN){
								enter_have_header(frame_len);
							} else {
								enter_have_header(32);
								//enter_search();
							}
							break;
						}
					}
				}
				else if(d_chip_num == 4){
					if(d_chip_cnt == 4+2){
						d_chip_cnt = 0;
						unsigned char c = decode_chips(d_shift_reg);

						if(c == 0xFF){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							//enter_search();
							//break;
						}

						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte which represents the frame length.
							int frame_len = d_packet_byte;
							frame_len = 32;
							if(frame_len <= MAX_PKT_LEN){
								enter_have_header(frame_len);
							} else {
								enter_have_header(32);
								//enter_search();
							}
							break;
						}
					}

				}
				else{
					if(d_chip_cnt == 64){
						d_chip_cnt = 0;
						unsigned char c = decode_chips(d_shift_reg);
						//fprintf(stream2,"%d\n",c);

						if(c == 0xFF){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							//enter_search();
							//break;
						}

						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte which represents the frame length.
							int frame_len = d_packet_byte;
							if(frame_len <= MAX_PKT_LEN){
								enter_have_header(frame_len);
							} else {
								enter_have_header(32);
								//enter_search();
							}
							break;
						}
					}
				}
				//my revision ends
			}
			break;

		case STATE_HAVE_HEADER:
			struct timeval a;
			gettimeofday(&a,NULL);
			fprintf(stderr,"==========\n");
			fprintf(stderr,"%lld\n",a.tv_usec);
			fprintf(stderr,"~~~~~~~~~~\n");

			//fprintf(stderr,"threshold:%d\td_chip_num:%d\t",d_threshold,d_chip_num);
			have_header = 1;
			if (VERBOSE2)
				fprintf(stderr,"Packet Build count=%d, ninput=%d, packet_len=%d\n", count, ninput, d_packetlen),fflush(stderr);
			while (count < ninput) {   // shift bits into bytes of packet one at a time				
				if(slice(inbuf[count])){
					d_shift_reg = (d_shift_reg << 1) | 1;
					compute_chip(inbuf[count],d_chip_cnt);	
					count++;
					//fprintf(stderr,"%f\n",inbuf[count-1]);
				}
				else{
					d_shift_reg = d_shift_reg << 1;
					compute_chip(inbuf[count],d_chip_cnt);
					count++;
					//fprintf(stderr,"%f\n",inbuf[count-1]);
				}
				if(d_chip_num == 32){
					d_chip_cnt = (d_chip_cnt+1)%32;
		
					if(d_chip_cnt == 0){
						unsigned char c = decode_chips(d_shift_reg);
						receive_symbol(d_shift_reg,c);				
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);
							c = 0;
						//	enter_search();
						//	break;
						}
						// the first symbol represents the first part of the byte.
						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						//fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte
							if (VERBOSE2)
								fprintf(stderr, "packetcnt: %d, payloadcnt: %d, payload 0x%x, d_packet_byte_index: %d\n", d_packetlen_cnt, d_payload_cnt, d_packet_byte, d_packet_byte_index), fflush(stderr);

							d_packet[d_packetlen_cnt++] = d_packet_byte;
							d_payload_cnt++;
							d_packet_byte_index = 0;

							if (d_payload_cnt >= d_packetlen){	// packet is filled, including CRC. might do check later in here
								receive_frame();
								
								
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);
								//new_chip_num = 16;
								
								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
							
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);
								//my code
								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));
								//fprintf(stderr,"Error chip number is: %d\n",error_count);
								struct timeval a;
								gettimeofday(&a,NULL);
								fprintf(stderr,"!!!!!!!!!!\n");
								fprintf(stderr,"%lld\n",a.tv_usec);
								fprintf(stderr,";;;;;;;;;;\n");

								if(VERBOSE2)
									fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
								enter_search();
								break;
							}
					
						}
					}
				}
				//my revision
				else if(d_chip_num == 16){
					
					d_chip_cnt = (d_chip_cnt+1)%16;
				
					if(d_chip_cnt == 0){
					//	fprintf(stderr,"%d\n",ce_preamble);
						unsigned char c = decode_chips(d_shift_reg);	
						receive_symbol(d_shift_reg,c);
						if(c == 0xff){
							dumped++;
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);
							c=0;
					//		enter_search();
					//		break;
						}
						// the first symbol represents the first part of the byte.
						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						//fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte
							if (VERBOSE2)
								fprintf(stderr, "packetcnt: %d, payloadcnt: %d, payload 0x%x, d_packet_byte_index: %d\n", d_packetlen_cnt, d_payload_cnt, d_packet_byte, d_packet_byte_index), fflush(stderr);

							d_packet[d_packetlen_cnt++] = d_packet_byte;
							d_payload_cnt++;
							d_packet_byte_index = 0;

							if (d_payload_cnt >= d_packetlen){	// packet is filled, including CRC. might do check later in here
								receive_frame();
								
								
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));
								//fprintf(stderr,"Error chip number is: %d\n",error_count);
								//FILE *f = fopen("/home/captain/experiment/cer-estimation/received","a+");
								//fclose(f);

								if(VERBOSE2)
									fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
								enter_search();
								break;
							}
					
						}
					}
				}
				else if(d_chip_num == 4){
					d_chip_cnt = (d_chip_cnt+1)%6;
				
					if(d_chip_cnt == 0){
						unsigned char c = decode_chips(d_shift_reg);
						receive_symbol(d_shift_reg,c);
						if(c == 0xff){
							dumped++;
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);
							c = 0;
							//enter_search();
							//break;
						}
						// the first symbol represents the first part of the byte.
						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						//fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte
							if (VERBOSE2)
								fprintf(stderr, "packetcnt: %d, payloadcnt: %d, payload 0x%x, d_packet_byte_index: %d\n", d_packetlen_cnt, d_payload_cnt, d_packet_byte, d_packet_byte_index), fflush(stderr);

							d_packet[d_packetlen_cnt++] = d_packet_byte;
							d_payload_cnt++;
							d_packet_byte_index = 0;

							if (d_payload_cnt >= d_packetlen){	// packet is filled, including CRC. might do check later in here
								receive_frame();
								
								
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));
								//fprintf(stderr,"Error chip number is: %d\n",error_count);
							//	FILE *f = fopen("/home/captain/experiment/cer-estimation/received","a+");
								//fclose(f);

								if(VERBOSE2)
									fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
								enter_search();
								break;
							}
					
						}
					}
				}
				else{

					d_chip_cnt = (d_chip_cnt+1)%64;
				
					if(d_chip_cnt == 0){
						bit_count += 64;
						unsigned char c = decode_chips(d_shift_reg);
						receive_symbol(d_shift_reg,c);
					
						
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %llx, ori: %llx\n", d_shift_reg,CHIP_MAPPING_4[8]), fflush(stderr);
							c = 0;
							//enter_search();
							//break;
						}
						// the first symbol represents the first part of the byte.
						if(d_packet_byte_index == 0){
							d_packet_byte = c;
						} else {
							// c is always < 15
							d_packet_byte |= c << 4;
						}
						//fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
						d_packet_byte_index = d_packet_byte_index + 1;
						if(d_packet_byte_index%2 == 0){
							// we have a complete byte
							if (VERBOSE2)
								fprintf(stderr, "packetcnt: %d, payloadcnt: %d, payload 0x%x, d_packet_byte_index: %d\n", d_packetlen_cnt, d_payload_cnt, d_packet_byte, d_packet_byte_index), fflush(stderr);

							d_packet[d_packetlen_cnt++] = d_packet_byte;
							d_payload_cnt++;
							d_packet_byte_index = 0;

							if (d_payload_cnt >= d_packetlen){	// packet is filled, including CRC. might do check later in here
								receive_frame();
								
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));
								//FILE *f = fopen("/home/captain/experiment/cer-estimation/received","a+");
							//	fclose(f);

								//fprintf(stderr,"Error chip number is: %d\n",error_count);

								if(VERBOSE2)
									fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
								enter_search();
								break;
							}
					
						}
					}
				}
				//my revision ends
			}

			//ber check will be put here
			break;

		default:
			assert(0);
			break;

		}
	}
	if(VERBOSE2)
		fprintf(stderr, "Samples Processed: %d\n", ninput_items[0]), fflush(stderr);

        consume(0, ninput_items[0]);
	//fclose(sequence);
	return 0;
}

private:
	enum {STATE_SYNC_SEARCH, STATE_HAVE_SYNC, STATE_HAVE_HEADER} d_state;

	unsigned int      d_sync_vector;           // 802.15.4 standard is 4x 0 bytes and 1x0xA7
	unsigned int      d_threshold;             // how many bits may be wrong in sync vector

	unsigned long long int      d_shift_reg;             // used to look for sync_vector
	int               d_preamble_cnt;          // count on where we are in preamble
	int               d_chip_cnt;              // counts the chips collected

	unsigned long long int      d_header;                // header bits
	int               d_headerbitlen_cnt;      // how many so far

	unsigned char     d_packet[MAX_PKT_LEN];   // assembled payload
	unsigned char     d_packet_byte;           // byte being assembled
	int               d_packet_byte_index;     // which bit of d_packet_byte we're working on
	int               d_packetlen;             // length of packet
	int               d_packetlen_cnt;         // how many so far
	int               d_payload_cnt;           // how many bytes in payload

	unsigned int      d_lqi;                   // Link Quality Information
	unsigned int      d_lqi_sample_count;

	unsigned int	  have_header;
	unsigned int	  d_chip_num;			   //chip_num is an optional variable, here we set it manually
	double			  bit_error_rate;			//indicate BER per packet
	unsigned int	  error_count;			  //errors per packet
	unsigned int	  bit_count;				//bit count in a data stream
	unsigned int	  ber_threshold;			//over the threshold we should lower the d_chip_num
	
	int				  d_head_threshold;	
	FILE*			  received;
	FILE*			  sequence;
	FILE*			  estimated;
	int				  ce_preamble;
	double			  per_chip_0_pos[64];
	double			  per_bit_pos[16];
	int				  chip_ct;
	int				  bct;
	double					be[256];
	FILE*			  received_be;
	int				  chip_errors;
	int				sym_cnt;
	double			w1;
	double			w2;
	double			ber_cost_upperbound;
	double			ber_cost;
	double			be1[4];
	double			be2[4];
	int				est_threshold[4];
	int				dumped;
	// FIXME:
	char buf[256];
};

packet_sink::sptr packet_sink::make(unsigned int threshold,unsigned int chip_num) {
	return gnuradio::get_initial_sptr(new packet_sink_impl(threshold,chip_num));
}
