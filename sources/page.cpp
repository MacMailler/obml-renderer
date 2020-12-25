#include "main.hpp"

namespace obml_renderer {
	page::page(const path& target) : _path(target) {
		load(target);
	}

	page::~page() {
		cleanup();
	}

	void page::load(const path& target) {
		cleanup();

		parser _parser(*this);
		_err = _parser.parse();
	}

	void page::cleanup() {
		_tiles.clear();
		_links.clear();
		_images.clear();

		_data.texts.clear();
		_data.tiles.clear();
	}

	void page::prepare() {
		_data.tiles.clear();
		_data.texts.clear();

#if defined __Debug__
		std::cout << "  --- links[" << _links.size() << "] ---" << std::endl;
#ifdef __DebugVerbose__
		for (const auto& i : _links) {
			std::cout
				<< "    position: " << i.regions.front().top << "x" << i.regions.front().left << std::endl
				<< "    size: " << i.regions.front().width << "x" << i.regions.front().height << std::endl
				<< "    type: " << i.target.type << std::endl
				<< "    target: " << i.target.href << std::endl << std::endl
			;
		}
#endif
#endif

#if defined __Debug__
		std::cout << "  --- images[" << _links.size() << "] ---" << std::endl;
#ifdef __DebugVerbose__
		for (const auto& i : _images) {
			std::cout
				<< "    addr: " << std::hex << i.first << std::dec << std::endl
				<< "    size: " << i.second->getSize().x << "x" << i.second->getSize().y << std::endl
				<< std::endl
			;
		}
#endif
#endif

#if defined __Debug__
		std::cout << "  --- tiles[" << _tiles.size() << "] ---" << std::endl;
#endif
		for (auto i : _tiles) {
			if (std::holds_alternative<tile>(i)) {
				const tile& t = std::get<tile>(i);

				sf::RectangleShape rect{{ t.bounds.width, t.bounds.height }};

				rect.setPosition(t.bounds.left, t.bounds.top);
				rect.setFillColor(t.color);

				_data.tiles.push_back(rect);

#ifdef __DebugVerbose__
				std::cout
					<< "    type: TILE" << std::endl
					<< "    position: " << t.bounds.left << "x" << t.bounds.top << std::endl
					<< "    size: " << t.bounds.width << "x" << t.bounds.height << std::endl
					<< "    color: " << std::hex << t.color.toInteger() << std::dec << std::endl
					<< std::endl
					;
#endif
			}
			else if (std::holds_alternative<image>(i)) {
				const image& j = std::get<image>(i);

				sf::RectangleShape rect{ { j.bounds.width, j.bounds.height } };
				rect.setPosition(j.bounds.left, j.bounds.top);
#if defined __DebugVerbose__
				std::cout
					<< "    type: IMAGE" << std::endl
					<< "    position: " << j.bounds.left << "x" << j.bounds.top << std::endl
					<< "    size: " << j.bounds.width << "x" << j.bounds.width << std::endl
					<< "    color: " << std::hex << j.color.toInteger() << std::dec << std::endl
					<< "    addr: " << std::hex << j.addr << std::dec
					;
#endif

				auto ik = _images.find(j.addr);
				if (ik == _images.end()) {
					rect.setFillColor(j.color);
#if defined __DebugVerbose__
					std::cout << " (!)";
#endif
				}
				else {
					if (ik->second == nullptr)
						rect.setFillColor(j.color);
					else
						rect.setTexture(ik->second.get());
				}

#if defined __DebugVerbose__
				std::wcout << std::endl << std::endl;
#endif
				_data.tiles.push_back(rect);
			}
			else if (std::holds_alternative<text>(i)) {
				const text& t = std::get<text>(i);

				sf::Text text{
					sf::String::fromUtf8(t.data.begin(), t.data.end()),
					_data._fonts->font,
					_data._fonts->font_sizes[t.font].size
				};

				text.setFillColor(t.color);
				text.setPosition(t.bounds.left, t.bounds.top);
				text.setStyle(_data._fonts->font_sizes[t.font].style);

				_data.texts.push_back(text);

#if defined __DebugVerbose__
				std::cout
					<< "    type: TEXT" << std::endl
					<< "    position: " << t.bounds.left << "x" << t.bounds.top << std::endl
					<< "    size: " << t.bounds.width << "x" << t.bounds.height << std::endl
					<< "    color: " << std::hex << t.color.toInteger() << std::dec << std::endl
					<< "    font: " << static_cast<int>(t.font) << std::endl
					<< "    text: " << t.data << std::endl << std::endl
					;
#endif
			}
			else if (std::holds_alternative<form>(i)) {
				form& f = std::get<form>(i);
#if defined __DebugVerbose__
				std::cout
					<< "    type: FORM" << std::endl
					<< "    position: " << f.bounds.left << "x" << f.bounds.top << std::endl
					<< "    size: " << f.bounds.width << "x" << f.bounds.height << std::endl
					<< "    color: " << std::hex << f.color.toInteger() << std::dec << std::endl
					<< "    type: " << f.type << std::endl
					<< "    id: " << f.id << std::endl
					<< "    value: " << f.value << std::endl << std::endl << std::endl
					;
#endif
			}
		}
#if defined __Debug__
		std::wcout << "\n";
#endif
	}

	void page::render() {
		_data.rt.create(
			_header.size.x,
			std::min(
				sf::Uint32(_header.size.y),
				sf::Texture::getMaximumSize()

			)
		);

		_data.rt.clear(sf::Color::White);

		for (auto i : _data.tiles)
			_data.rt.draw(i);

		for (auto i : _data.texts)
			_data.rt.draw(i);

		_data.rt.display();
	}

	void page::render(sf::RenderTarget& target) {
		target.clear();

		for (auto i : _data.tiles)
			target.draw(i);

		for (auto i : _data.texts)
			target.draw(i);
	}

	void page::update_fonts() {
		_data.texts.clear();

		for(auto &i : _tiles) if (std::holds_alternative<text>(i)) {
			const text& t = std::get<text>(i);

			sf::Text text{
				sf::String::fromUtf8(t.data.begin(), t.data.end()),
				_data._fonts->font,
				_data._fonts->font_sizes[t.font].size
			};

			text.setFillColor(t.color);
			text.setPosition(t.bounds.left, t.bounds.top);
			text.setStyle(_data._fonts->font_sizes[t.font].style);

			_data.texts.push_back(text);
		}
	}

	void page::set_fonts(sptr_t<render::fonts>& fonts) {
		_data._fonts = fonts;
	}

	header& page::get_header() {
		return _header;
	}

	tiles& page::get_tiles() {
		return _tiles;
	}

	links& page::get_links() {
		return _links;
	}

	images& page::get_images() {
		return _images;
	}

	const path& page::get_path() const {
		return _path;
	}

	int page::get_err() const {
		return _err;
	}

	const sf::Texture& page::get_texture() const {
		return _data.rt.getTexture();
	}

	bool page::export_page(const path& dest, const char* format) const {
		sf::Image& image = _data.rt.getTexture().copyToImage();

		std::stringstream ss;
		ss << dest << _path.stem() << "." << format;

#if defined __Debug__
		std::cout << "Exporting page to '" << ss.str() << "'" << std::endl;
#endif
		return image.saveToFile(ss.str());
	}

	bool page::export_region(const fs::path& dest, const sf::FloatRect& region, const char* format) const {
		sf::IntRect r{ region };

		sf::Sprite t{ _data.rt.getTexture(), r };

		sf::RenderTexture rt;
		rt.create(r.width, r.height);
		rt.clear(sf::Color::Transparent);

		rt.draw(t);
		rt.display();

		sf::Image& image = rt.getTexture().copyToImage();
		uint32_t checksum = 0;//crc32::update(crc32_table, 0, image.getPixelsPtr(), (rt.getSize().x * rt.getSize().y) * 4);

		std::stringstream ss;
		ss << dest.string() << _path.stem().u8string() << "_" << std::hex << checksum << "." << format;

#if defined __Debug__
		std::cout << "Exporting region to '" << ss.str() << "'" << std::endl;
#endif
		return image.saveToFile(ss.str());
	}

	void page::export_images(const path& dest) const {
		sf::Int32 count = 0;
		std::stringstream fmt;

		for (const auto& i : _images) {
			fmt.clear();

			fmt << dest << "File" << count++ << ".jpeg";

			i.second->copyToImage().saveToFile(fmt.str());
		}
	}
}