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
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
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
}

void reset_ber()
{
	bit_count = 0;
	error_count = 0;
	bit_error_rate = 0;
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
			threshold = gr::blocks::count_bits32((chips & 0x6) ^( CHIP_MAPPING_3[i] & 0x6));
		else
			threshold = gr::blocks::count_bits64((chips & 0x7FFFFFFFFFFFFFFE) ^( CHIP_MAPPING_4[i] & 0x7FFFFFFFFFFFFFFE));
		//revision ends
		if (threshold < min_threshold) {
			best_match = i;
			min_threshold = threshold;
		}
	}

	flag = false;
	if (min_threshold < d_threshold) {
		error_count += count_error(chips,d_chip_num,best_match);
		if (flag && d_chip_num == 32){
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[best_match] & 0x7FFFFFFE)), fflush(stderr);
		// LQI: Average number of chips correct * MAX_LQI_SAMPLES
		}else if(flag && d_chip_num == 16){
				fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x7FFE) ^ (CHIP_MAPPING_2[best_match] & 0x7FFE)), fflush(stderr);
		}else if(flag && d_chip_num == 8){
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips & 0x7E) ^ (CHIP_MAPPING_3[best_match] & 0x7E)), fflush(stderr);			
		}else if(flag && d_chip_num == 64){
			fprintf(stderr, "Found sequence with %d errors at 0x%x, send: %x,receive:%x\n", min_threshold, (chips & 0x7FFFFFFFFFFFFFF0) ^ (CHIP_MAPPING_4[best_match]), fflush(stderr));
		}
		if (d_lqi_sample_count < MAX_LQI_SAMPLES) {
			d_lqi += d_chip_num - min_threshold;//It is 32 originally
			d_lqi_sample_count++;
		}
		return (char)best_match & 0xF;
	}
	else{
		error_count += d_chip_num;
	}
	return 0xFF;
}

unsigned int count_error(long long int chips,int chip_num, int match){
	if(chip_num == 64)
		return gr::blocks::count_bits64((chips & 0x7FFFFFFFFFFFFFFE) ^ (CHIP_MAPPING_4[match] & 0x7FFFFFFFFFFFFFFE));
	else if(chip_num == 32)
		return gr::blocks::count_bits32((chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[match] & 0x7FFFFFFE));
	else if(chip_num == 16)	
		return gr::blocks::count_bits16((chips & 0x7FFE) ^ (CHIP_MAPPING_2[match] & 0x7FFE));
	else if(chip_num == 4)	
		return gr::blocks::count_bits8((chips & 0x6) ^ (CHIP_MAPPING_3[match] & 0x6));
}

int slice(float x) {
	return x > 0 ? 1 : 0;
}

double get_ber_value(){
	return double(error_count)/bit_count;
}

int8_t feedback_new_chip_num(int options){
	//option = 0: increase optiaon = 1:decrease
	//decrease part
	if(options == 0){
		//increase by 1
	}
	else{
		//decrease by 1
	}
//	message_port_pub(pmt::mp("feedback out"),32);
	//increase part
	return 32;
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

	if ( VERBOSE )
		fprintf(stderr, "syncvec: %x, threshold: %d\n", d_sync_vector, d_threshold),fflush(stderr);
	enter_search();

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

	if (VERBOSE)
		fprintf(stderr,">>> Entering state machine\n"),fflush(stderr);
	while(count < ninput) {
		switch(d_state) {

		case STATE_SYNC_SEARCH:    // Look for sync vector
			have_header = 0;
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
						threshold = gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[0] & 0x7FFFFFFE));
						if(threshold < d_threshold) {
							if (VERBOSE2)
								fprintf(stderr,"Found 0 in chip sequence\n"),fflush(stderr);
							// we found a 0 in the chip sequence
							d_preamble_cnt+=1;
							//fprintf(stderr, "Threshold %d d_preamble_cnt: %d\n", threshold, d_preamble_cnt);						
						}
					} else {
						// we found the first 0, thus we only have to do the calculation every 32 chips
						if(d_chip_cnt == 32){
							d_chip_cnt = 0;
							if(d_packet_byte == 0) {
								if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[0] & 0xFFFFFFFE)) <= d_threshold) {
									if (VERBOSE2)
										fprintf(stderr,"Found %d 0 in chip sequence\n", d_preamble_cnt),fflush(stderr);
									// we found an other 0 in the chip sequence
									d_packet_byte = 0;
									d_preamble_cnt ++;
								}else if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[7] & 0xFFFFFFFE)) <= d_threshold) {
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
								if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[10] & 0xFFFFFFFE)) <= d_threshold
										) {
									d_packet_byte |= 0xA;
									d_chip_num = 64;
									if (VERBOSE2)
										fprintf(stderr,"Found sync, 0x%x\n", d_packet_byte),fflush(stderr);
									// found SDF
									// setup for header decode
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[11] & 0xFFFFFFFE)) <= d_threshold){
									d_packet_byte |= 0xB;
									d_chip_num = 32;
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[12] & 0xFFFFFFFE)) <= d_threshold){
									d_packet_byte |= 0xC;
									d_chip_num = 16;
									enter_have_sync();
									break;
								}
								else if (gr::blocks::count_bits32((d_shift_reg & 0x7FFFFFFE) ^ (CHIP_MAPPING[13] & 0xFFFFFFFE)) <= d_threshold){
									d_packet_byte |= 0xD;
									d_chip_num = 4;
									enter_have_sync();
									break;
								}
								 else {
									if (VERBOSE)
										fprintf(stderr, "Wrong second byte of SFD. %u\n", d_shift_reg), fflush(stderr);
									enter_search();
									break;
								}
							}
						}
					
					}//d_premble_cnt == 0 ends
				//my revision
			}//while ends
			break;

		case STATE_HAVE_SYNC:
			have_header = 0;
			reset_ber();
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

							enter_search();
							break;
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
								enter_search();
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

							enter_search();
							break;
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
								enter_search();
							}
							break;
						}
					}
				}
				else if(d_chip_num == 4){
					if(d_chip_cnt == 4){
						d_chip_cnt = 0;
						unsigned char c = decode_chips(d_shift_reg);
						//fprintf(stream2,"%d\n",c);

						if(c == 0xFF){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							enter_search();
							break;
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
								enter_search();
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

							enter_search();
							break;
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
								enter_search();
							}
							break;
						}
					}
				}
				//my revision ends
			}
			break;

		case STATE_HAVE_HEADER:
			have_header = 1;
			if (VERBOSE2)
				fprintf(stderr,"Packet Build count=%d, ninput=%d, packet_len=%d\n", count, ninput, d_packetlen),fflush(stderr);

			while (count < ninput) {   // shift bits into bytes of packet one at a time				
				if(slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;
				
				if(d_chip_num == 32){
					d_chip_cnt = (d_chip_cnt+1)%32;
				
					if(d_chip_cnt == 0){
						bit_count += 32;
						unsigned char c = decode_chips(d_shift_reg);
						
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							enter_search();
							break;
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
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);
								//new_chip_num = 16;
								
								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
							
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);
								//my code

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));

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
						bit_count += 16;
						unsigned char c = decode_chips(d_shift_reg);
					//	fprintf(stream2, "%d\n",c);
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							enter_search();
							break;
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
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));

								if(VERBOSE2)
									fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
								enter_search();
								break;
							}
					
						}
					}
				}
				else if(d_chip_num == 4){
					d_chip_cnt = (d_chip_cnt+1)%4;
				
					if(d_chip_cnt == 0){
						bit_count += 4;
						unsigned char c = decode_chips(d_shift_reg);
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							enter_search();
							break;
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
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));

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
					//	fprintf(stream2, "%d\n",c);
						if(c == 0xff){
							// something is wrong. restart the search for a sync
							if(VERBOSE2)
								fprintf(stderr, "Found a not valid chip sequence! %u\n", d_shift_reg), fflush(stderr);

							enter_search();
							break;
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
								unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES) << 3;
								unsigned char lqi = (scaled_lqi >= 256? 255 : scaled_lqi);

								pmt::pmt_t meta = pmt::make_dict();
								meta = pmt::dict_add(meta, pmt::mp("lqi"), pmt::from_long(lqi));

								std::memcpy(buf, d_packet, d_packetlen_cnt);
								pmt::pmt_t payload = pmt::make_blob(buf, d_packetlen_cnt);

								message_port_pub(pmt::mp("out"), pmt::cons(meta, payload));

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
		//	if(get_ber_value() > ber_threshold)
		//	fprintf(stderr,"header\n");
//			new_chip_num = feedback_new_chip_num(0);
		//	new_chip_num = 16;
//			feedback_out[0] = new_chip_num;
		//	else if("time % 100 = 0")
			break;

		default:
			assert(0);
			break;

		}
	}
	if(VERBOSE2)
		fprintf(stderr, "Samples Processed: %d\n", ninput_items[0]), fflush(stderr);

        consume(0, ninput_items[0]);
	if(new_chip_num != 0) 
	{	
		FILE *ack_file = fopen("/home/captain/test/receiver/ack","w+");
		fprintf(ack_file,"%d",new_chip_num);
		fclose(ack_file);	
	}


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
	// FIXME:
	char buf[256];
};

packet_sink::sptr packet_sink::make(unsigned int threshold,unsigned int chip_num) {
	return gnuradio::get_initial_sptr(new packet_sink_impl(threshold,chip_num));
}
