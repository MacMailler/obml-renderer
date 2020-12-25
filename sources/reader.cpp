#include "main.hpp"

namespace obml_renderer {
	reader::reader(const path& _path) {
		open(_path);
	}

	reader::reader() {
	}
	
	reader::~reader() {
		close();
	}

	void reader::open(const path& _path) {
		_fh.open(_path, std::ios::in | std::ios::binary);
	}

	void reader::close() {
		_fh.close();
	}

	int8_t reader::read_byte() {
		char buf;
		_fh.read(&buf, sizeof buf);
		return buf;
	}

	int16_t reader::read_short() {
		char buf[2];
		_fh.read(buf, 2);
		return static_cast<int16_t>(
			((buf[0] & 0xFF) << 8) | (buf[1] & 0xFF)
		);
	}

	int32_t reader::read_medium() {
		char buf[3];
		_fh.read(buf, 3);
		return static_cast<int32_t>(
			(buf[0] << 16) | (((buf[1] & 0xFF) << 8) | (buf[2] & 0xFF))
		);
	}

	sf::Vector2f reader::read_coord() {
		return {
			static_cast<float>(read_short()),
			static_cast<float>(read_medium())
		};
	}

	sf::Color reader::read_color() {
		uint8_t dest[4] = { 0, 0, 0, 0 };
		_fh.read(reinterpret_cast<char*>(&dest), sizeof dest);

		return {
			dest[1],
			dest[2],
			dest[3],
			dest[0]
		};
	}

	std::string reader::read_url() {
		auto len = read_short();

		if (read_byte() != '\0')
			_fh.seekg(-1, std::ios::cur);
		else
			len--;

		std::string buf(len, 0);
		_fh.read(&buf[0], len);

		return buf;
	}

	std::string reader::read_string() {
		auto len = read_short();
		std::string buf;

		if (len > 0) {
			buf.resize(len);
			_fh.read(&buf[0], len);
		}

		return buf;
	}

	uptr_t<blob> reader::read_blob() {
		auto len = read_short();
		uptr_t<blob> ret = nullptr;

		if (len > 0) {
			ret = std::make_unique<blob>(len + 1, 0);
			_fh.read(reinterpret_cast<char*>(ret->data()), len);
		}

		return std::move(ret);
	}

	uptr_t<sf::Texture> reader::read_image() {
		auto len = read_short();
		uptr_t<sf::Texture> ret = nullptr;

		if (len > 0) {
			auto buf = std::make_unique<char[]>(len);
			if (_fh.read(buf.get(), len).good()) {
				ret = std::make_unique<sf::Texture>();
				ret->loadFromMemory(buf.get(), len);
			}
		}

		return std::move(ret);
	}

	void reader::dump(size_t bytes, const path& _path) {
		blob buf(bytes);
		_fh.read(&buf[0], bytes);

		std::ofstream _out(_path, std::ios::out | std::ios::trunc | std::ios::binary);
		_out.write(&buf[0], buf.size());
		_out.close();
	}

	void reader::dump_blob(const path& _path) {
		dump(read_short(), _path);
	}

	void reader::dump_blob_alt(const path& _path) {
		dump(read_medium(), _path);
	}

	size_t reader::tell() {
		return static_cast<size_t>(_fh.tellg());
	}

	size_t reader::skip(size_t bytes) {
		return static_cast<size_t>(_fh.ignore(bytes).tellg());
	}

	size_t reader::skip_blob() {
		auto len = read_short();
		return skip(len);
	}

	size_t reader::skip_blob_alt() {
		auto len = read_medium();
		return skip(len);
	}

	std::fstream& reader::get_handle() {
		return _fh;
	}
};