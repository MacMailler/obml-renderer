#pragma once

#include "main.hpp"

namespace obml_renderer {
	class reader {
	public:
		reader(const path& _path);
		reader();
		~reader();

		void open(const path& _path);
		bool is_open();
		void close();

		int8_t read_byte();
		int16_t read_short();
		int32_t read_medium();

		sf::Vector2f read_coord();
		sf::Color read_color();

		std::string read_url();
		std::string read_string();

		uptr_t<blob> read_blob();
		uptr_t<sf::Texture> read_image();

		void dump(size_t bytes, const path& _path);
		void dump_blob(const path& _path);
		void dump_blob_alt(const path& _path);

		size_t tell();

		size_t skip(size_t bytes);
		size_t skip_blob();
		size_t skip_blob_alt();

		std::fstream& get_handle();

	private:
		std::fstream _fh;
	};
};
