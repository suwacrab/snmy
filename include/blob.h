#ifndef BLOB_H
#define BLOB_H

#include <vector>
#include <string>
#include <cstdint>

class Blob {
	private:
		std::vector<uint8_t> m_data;
	public:
		void write_blob(Blob& orig);
		void write_raw(const void* source, size_t len);
		void write_u8(uint32_t n);
		void write_u16(uint32_t n);
		void write_u32(uint32_t n);

		void write_str(const std::string& str, bool no_terminator=false);
		auto hash() const -> uint32_t;

		void reset();
		bool send_file(std::string filename, bool strict=true);

		template<typename T=void> auto data() -> T* {
			return reinterpret_cast<T*>(m_data.data());
		}
		constexpr auto size() const -> size_t { return m_data.size(); }

		Blob();
		Blob(Blob& orig);
};

#endif

