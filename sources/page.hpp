#pragma once

#include "main.hpp"

namespace obml_renderer {
	class parser;

	namespace render {
		struct font_style {
			uint32_t size;
			uint8_t style;
		};

		struct fonts {
			sf::Font font;
			std::map<int8_t, font_style> font_sizes = {
				{ 2,{ 14, sf::Text::Style::Regular } },		// medium
				{ 3,{ 14, sf::Text::Style::Bold } },		// medium bold
				{ 4,{ 20, sf::Text::Style::Regular } },		// large
				{ 5,{ 20, sf::Text::Style::Bold } },		// large bold
				{ 6,{ 12, sf::Text::Style::Regular } }		// small
			};
		};

		struct data {
			sf::RenderTexture rt;

			std::list<sf::RectangleShape> tiles;
			std::list<sf::Text> texts;

			sptr_t<fonts> _fonts;
		};
	};

	class page : private sf::NonCopyable {
	public:
		explicit page(const path& target);
		~page();
	
		void prepare();
		void render();
		void render(sf::RenderTarget& target);

		void load(const path& target);
		void cleanup();

		void update_fonts();
		void set_fonts(sptr_t<render::fonts>& fonts);

		header& get_header();
		images& get_images();
		tiles& get_tiles();
		links& get_links();

		int get_err() const;
		const path& get_path() const;
		const sf::Texture& get_texture() const;

		bool export_page(const path& dest, const char* format = "png") const;
		bool export_region(const path& dest, const sf::FloatRect& region, const char* format = "png") const;
		void export_images(const path& dest) const;

	private:
		header _header;
		tiles _tiles;
		links _links;
		images _images;

		render::data _data;

		path _path;
		int _err;
	};
};