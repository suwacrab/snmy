/*
	*	each line begins with a 16-bit header.
		-	high 2 bits: the compression mode
		-	low 14 bits: length of whatever data the line'll use,, minus 1.
	*	after the header, the data varies depending on the mode.
	*	3 compression modes in total
		-	0: no compression
			data is simply raw bytes immediately after the header
		-	1: compressed (RLE)
			1 byte for the run.
		-	2: compressed (LZ)
			2 bytes for the offset to go backwards.(minus one)
		-	3: end code
			compression should end here.
*/

#include <cstdio>
#include <snmy.h>

static std::vector<uint8_t> file_loadToVec(FILE *file) {
	size_t filesize = 0;
	std::fseek(file,0,SEEK_END);
	filesize = std::ftell(file);
	std::rewind(file);

	// read entire file to vector -------------------@/
	std::vector<uint8_t> filebuf(filesize);
	std::fread(filebuf.data(),1,filesize,file);
	return filebuf;
}

namespace snmy {
	auto compress_file(const std::string& filename, const snmy::CCompressInfo* info) -> Blob {
		// open file & get size -------------------------@/
		auto file = std::fopen(filename.c_str(),"rb");
		if(!file) {
			std::printf("snmy::compress_file(): error: unable to open file '%s' for reading\n",
				filename.c_str()
			);
			std::exit(-1);
		}
		auto filebuf = file_loadToVec(file);
		std::fclose(file);
		
		return snmy::compress(filebuf,info);
	}
	auto compress(const std::vector<uint8_t>& src_buffer, const snmy::CCompressInfo* info) -> Blob {
		bool do_verbose = false;
		if(info) do_verbose = info->verbose;

		Blob out_blob;
		const size_t src_size = src_buffer.size();

		int stat_matchRaw = 0;
		int stat_matchRL = 0;
		int stat_matchLZ = 0;

		int matchRaw_size = 0;
		int matchRaw_start = 0;
		for(int src_index=0; src_index<src_size;) {
			int matchRL_size = 0;
			int matchRL_startchar = 0;
			int matchLZ_offset = 0;
			int matchLZ_size = 0;
			
			// search for RLE match
			matchRL_startchar = src_buffer.at(src_index);
			for(int i=src_index; i<src_size; i++) {
				if(matchRL_size >= snmy::DATALINE_MAX) break;
				auto chr = src_buffer.at(i);
				if(chr != matchRL_startchar) break;
				matchRL_size++;
			}

			// search for LZ match
			for(int i=1; i<32768; i++) {
				if(i > src_index) break;
				const int lz_maxlen = i;
				int lz_start = src_index - i;
				int lz_curSize = 0;
				for(int j=0; j<lz_maxlen; j++) {
					if(lz_curSize >= DATALINE_MAX) break;
					if(j+src_index >= src_size) break;
					if(src_buffer.at(lz_start+j) != src_buffer.at(src_index+j)) break;
					lz_curSize++;
				}

				// update previous largest LZ match -----@/
				if(lz_curSize >= matchLZ_size) {
					matchLZ_size = lz_curSize;
					matchLZ_offset = i;
				}
			}
			
			// write data depending on lines' best matches
			if(matchRL_size >= 3 || matchLZ_size >= 4) {
				// BUT, eject the raw data, first. ------@/
				if(matchRaw_size > 0) {
					int header = matchRaw_size-1;
					out_blob.write_u16(header);
					for(int i=0; i<matchRaw_size; i++) {
						out_blob.write_u8(src_buffer.at(matchRaw_start++));
					}
					stat_matchRaw++;
				}
				
				// then, deal with either run-length or LZ data
				if(matchRL_size >= matchLZ_size && matchRL_size >= 3) {
					src_index += matchRL_size;
					int header = 1 << 14;
					header |= (matchRL_size-1);
					out_blob.write_u16(header);
					out_blob.write_u8(matchRL_startchar);
					stat_matchRL++;
				} else if(matchLZ_size > matchRL_size && matchLZ_size >= 4) {
					src_index += matchLZ_size;
					int header = 2 << 14;
					header |= (matchLZ_size-1);
					out_blob.write_u16(header);
					out_blob.write_u16(matchLZ_offset-1);
					stat_matchLZ++;
				}
				matchRaw_size = 0;
				matchRaw_start = src_index;
			} else {
				src_index++;
				matchRaw_size++;
				if(matchRaw_size >= DATALINE_MAX) {
					int header = matchRaw_size-1;
					out_blob.write_u16(header);
					for(int i=0; i<matchRaw_size; i++) {
						out_blob.write_u8(src_buffer.at(matchRaw_start++));
					}
					stat_matchRaw++;
					matchRaw_size = 0;
					matchRaw_start = src_index;
				} 
			}
		}

		if(do_verbose) {
			std::printf("lines (raw): %4d\n",stat_matchRaw);
			std::printf("lines (rle): %4d\n",stat_matchRL);
			std::printf("lines (lz):  %4d\n",stat_matchLZ);
		}

		// final data -----------------------------------@/
		out_blob.write_u16(0xFFFF);
		return out_blob;
	}

	auto decompress_file(const std::string& filename) -> Blob {
		// open file & get size -------------------------@/
		auto file = std::fopen(filename.c_str(),"rb");
		if(!file) {
			std::printf("snmy::decompress_file(): error: unable to open file '%s' for reading\n",
				filename.c_str()
			);
			std::exit(-1);
		}
		auto filebuf = file_loadToVec(file);
		std::fclose(file);
		
		return snmy::decompress(filebuf);
	}
	auto decompress(const std::vector<uint8_t>& src_buffer) -> Blob {
		Blob out_blob;
		const size_t src_size = src_buffer.size();

		for(int src_index=0; src_index<src_size;) {
			// get header information -------------------@/
			int start_index = src_index;
			int header = 0;
			header |= src_buffer.at(start_index);
			header |= src_buffer.at(start_index+1) << 8;
			int header_mode = header >> 14;
			int header_length = (header & 0x3FFF) + 1;
	
			bool do_stopdecompress = false;

			// read depending on mode
			switch(header_mode) {
				// raw data -----------------------------@/
				case 0: {
					src_index += 2;
					for(int i=0; i<header_length; i++) {
						out_blob.write_u8(src_buffer.at(src_index++));
					}
					break;
				}
				// RLE line -----------------------------@/
				case 1: {
					src_index += 2;
					int run = src_buffer.at(src_index++);
					for(int i=0; i<header_length; i++) {
						out_blob.write_u8(run);
					}
					break;
				}
				// LZ line ------------------------------@/
				case 2: {
					src_index += 2;
					int offset = 0;
					offset |= src_buffer.at(src_index++);
					offset |= src_buffer.at(src_index++) << 8;
					offset += 1;
					int lz_start = out_blob.size() - offset;

					// temp
					for(int i=0; i<header_length; i++) {
						out_blob.write_u8(out_blob.data<uint8_t>()[lz_start+i]);
					}
					break;
				}
				// end code
				case 3: {
					do_stopdecompress = true;
					break;
				}
				default: {
					std::puts("snmy::decompress(): error: unknown decompression line mode");
					std::exit(-1);
					break;
				}
			}

			if(do_stopdecompress) break;
		}

		return out_blob;
	}
}

