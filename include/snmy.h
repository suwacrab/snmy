#ifndef SNMY_H
#define SNMY_H

#include <cstdint>
#include <string>
#include <vector>
#include <blob.h>

namespace snmy {
	constexpr size_t DATALINE_MAX = 16384;

	class CCompressInfo;

	auto compress_file(const std::string& filename, const snmy::CCompressInfo* info = NULL) -> Blob;
	auto compress(const std::vector<uint8_t>& src_buffer, const snmy::CCompressInfo* info) -> Blob;

	auto decompress_file(const std::string& filename) -> Blob;
	auto decompress(const std::vector<uint8_t>& src_buffer) -> Blob;
};

class snmy::CCompressInfo {
	public:
		CCompressInfo()
			: verbose(false)
			{}

		void verbose_set(bool flag) { verbose = flag; }
		int verbose;
};

#endif

