#include "main.hpp"

namespace obml_renderer {
	parser::parser(page& p) : _page(p) {
	}

	parser::~parser() {
	}

	parser::err parser::parse() {
		err _err = err::none;
		_reader.open(_page.get_path());

		if (!_reader.get_handle().is_open())
			_err = err::bad_path;

		if (_err == err::none)
			_err = read_header();

		if (_err == err::none)
			_err = read_metadata();

		if (_err == err::none)
			_err = read_links();

		if (_err == err::none)
			_err = read_content();

		return _err;
	}

	parser::err parser::read_header() {
		header& _header = _page.get_header();

		_header.data_len = _reader.read_medium() + 3;
		_header.version = _reader.read_byte();

		if (_header.version != ver::v6) {
			std::cout << "ERROR: OBML v" << (short)_header.version << " are not supported!" << std::endl;

			return err::version;
		}

		_header.size = static_cast<sf::Vector2i>(_reader.read_coord());

		// skip S\x00\x00\xFF\xFF
		_reader.skip(5);

		_header.title = _reader.read_string();

		// unknown: blob
		_reader.skip_blob();
		//_reader.dump_blob("header_unk.blob");
		
		_header.base_url = _reader.read_string();
		_header.page_url = _reader.read_string();

		// metadata section
		_reader.skip(1); // skip unknown: byte (always 19 or 23)

		return err::none;
	}

	parser::err parser::read_metadata() {
		while (_links_size == 0) {
			switch (_reader.read_byte()) {
			case 'M': {
				switch (_reader.read_byte()) {
				case 'C': // skip unknown: blob
					_reader.dump_blob_alt("metadata_unk.blob");
				//	_reader.skip_blob_alt();
					break;

				case 'u': // skip unknown: byte[7]
					_reader.skip(7);
					break;

				case 'S': // skip tls information
					_reader.skip_blob_alt();
					break;
				}
			}
					  break;

			case 'S': // links section
				_links_size = _reader.read_medium();
				break;
			}
		}

		// links section
		_links_begin = _reader.tell();
		_links_end = _links_begin + _links_size;

//		std::cout << "links_start=" << _links_begin << ", links_end=" << _links_size << std::endl;

		return err::none;
	}

	parser::err parser::read_links() {
		links& _links = _page.get_links();

		while (_reader.tell() < _links_end) {
			int8_t type = _reader.read_byte();
			switch (type) {
			case '\0': { // data for drop-down lists (strings)
				_reader.skip(1);
				for (int8_t i = 0, len = _reader.read_byte(); i < len; i++) {
					auto bytes = _reader.read_short();
					_reader.skip(bytes);
					_reader.skip_blob();
				}
			}
					   break;

			case 'i':
			case 'w':
			case 'W':
			case 'L':
			case 'P': {
				int8_t count = _reader.read_byte();
				if (count == 0)
					_reader.skip(8);
				else {
					link l{};

					for (int8_t i = 0; i < count; i++)
						l.regions.push_back({ _reader.read_coord(), _reader.read_coord() });

					l.target.type = _reader.read_string();
					l.target.href = _reader.read_url();

					_links.push_back(l);
				}
			}
					  break;

			case 'C':// skip unknown: byte[21]
				_reader.skip(21);
				break;

			case 'I': {
				link l{};
				for (int8_t i = 0, len = _reader.read_byte(); i < len; i++)
					l.regions.push_back({ _reader.read_coord(), _reader.read_coord() });

				_reader.skip_blob();
				_reader.skip(5);

				_links.push_back(l);
			}
					  break;

			case 'N':
			case 'S': {
				link l{};

				for (int8_t i = 0, len = _reader.read_byte(); i < len; i++)
					l.regions.push_back({ _reader.read_coord(), _reader.read_coord() });

				_reader.skip_blob(); // link_target: blob
				_reader.skip_blob(); // link_target: blob

				_links.push_back(l);
			}
			break;

			default:
				std::cout << "unknown link section at " << _reader.tell() << std::endl;
				return err::bad_link_tag;
			}
		}

		if (_reader.tell() != _links_end) {
			std::cout << "section ended at " << _reader.tell() << ", expected " << _links_end << std::endl;
			return err::unknown;
		}

		return err::none;
	}

	parser::err parser::read_content() {
		tiles& _tiles = _page.get_tiles();
		images& _images = _page.get_images();

		size_t content_end = _page.get_header().data_len;
//		std::cout << "content_start=" << _reader.tell() << ", content_end=" << content_end << std::endl;

		while (_reader.tell() < content_end) {
			int8_t type = _reader.read_byte();

			//std::cout << "[" << type << "]=" << _file.tell() << std::endl;

			switch (type) {
			case 'L':
				_reader.skip(9);
				break;

			case 'z':
				_reader.skip(6);
				break;

			case 'o':
				_reader.skip_blob();
				break;

			case 'M': {
				_reader.skip(2);
				_reader.skip_blob();
			}
					  break;

			case 'B': {
				_tiles.push_back(tile{
					{
						_reader.read_coord(),
						_reader.read_coord(),
					},
					_reader.read_color()
				});
			}
					  break;

			case 'I': {
				image i{};

				i.bounds = { _reader.read_coord(), _reader.read_coord() };
				i.color = _reader.read_color();
				_reader.skip(3);
				i.addr = _reader.read_medium();

				_tiles.push_back(i);
			}
					  break;

			case 'F': {
				form f{};

				f.bounds = { _reader.read_coord(), _reader.read_coord() };
				f.color = _reader.read_color();

				f.type = _reader.read_short();
				f.id = _reader.read_string();
				f.value = _reader.read_string();

				_reader.skip(3); // \xFF\xFF\xFF

				_tiles.push_back(f);
			}
					  break;

			case 'T': {
				text t{};

				t.bounds = { _reader.read_coord(), _reader.read_coord() };
				t.color = _reader.read_color();
				t.font = _reader.read_byte();
				t.data = _reader.read_string();

				_tiles.push_back(t);
			}
					  break;

			case 'S': {
				size_t data_size = _reader.read_medium();
				size_t data_begin = _reader.tell();
				size_t data_end = data_begin + data_size;

				//std::cout << "data_size=" << data_size << std::endl;

				while (_reader.tell() < data_end) {
					auto addr = _reader.tell() - 3;
					auto image = _reader.read_image();

					if (image != nullptr)
						_images.insert({ addr, std::move(image) });
					else
						return err::bad_data;
				}
			}
			break;

			default:
				return err::bad_content_tag;
			}
		}

		return err::none;
	}
};