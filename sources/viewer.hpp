#pragma once

#include "main.hpp"

#ifdef _WIN32
	#include <windows.h>
#endif

namespace obml_renderer {
	struct selector : sf::Drawable {
		static const sf::Color outline;
		static const sf::Color fill;

	private:
		sf::RectangleShape rect;
		link* region;

		bool has_show = false;

	protected:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	public:
		selector();
		void show(const sf::Vector2f& position, const sf::Vector2f& size, link* region = nullptr);
		void move(float x, float y);
		void hide();
		link* get_region();
	};

	struct scroll_info {
		sf::Vector2f position;
	};

	class viewer : private sf::NonCopyable {
	public:
		explicit viewer(const sf::VideoMode& mode);
		~viewer();

		void open();

	private:
		uptr_t<page> _page;

		sf::RenderWindow _window;
		sf::View _view;

		sptr_t<render::fonts> _fonts;
		scroll_info _scroll;
		selector _selector;

		sf::Vector2f _drawing_offset = { 0.f, 0.f };

		bool show_page_info = false;

		void setup_imgui();
		void draw_main_bar();
		void draw_info();
		void draw_tabs();

		void set_scroll_page_y(float amount, float factor = 64.f);
		void reset_scroll();

#ifdef _WIN32
		OPENFILENAME _ofn;
		std::wstring _path;
		void setup_openfilename();
#endif
	};
};