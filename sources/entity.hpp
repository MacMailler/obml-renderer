#pragma once

#include "main.hpp"

namespace obml_renderer {
	struct tile {
		sf::FloatRect bounds;
		sf::Color color;
	};

	struct image : tile {
		uint32_t addr{0};
	};

	struct text : tile {
		int8_t font;
		std::string data;
	};

	struct form : tile {
		int16_t type;
		std::string id;
		std::string value;
	};

	struct url {
		std::string type;
		std::string href;
	};

	struct link {
		std::list<sf::FloatRect> regions;
		url target;
	};

	struct header {
		uint32_t data_len;
		uint8_t version;

		sf::Vector2i size;

		std::string title;
		std::string base_url;
		std::string page_url;
	};

	namespace fs = std::experimental::filesystem::v1;

	template<typename T> using uptr_t = std::unique_ptr<T>;
	template<typename T> using sptr_t = std::shared_ptr<T>;

	using path = std::experimental::filesystem::v1::path;
	using images = std::unordered_map<uint32_t, uptr_t<sf::Texture>>;
	using tiles = std::list<std::variant<tile, image, text, form>>;
	using links = std::list<link>;
	using blob = std::vector<char>;
};