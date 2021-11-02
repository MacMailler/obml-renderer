#include "viewer.hpp"

namespace obml_renderer {
	viewer::viewer(const sf::VideoMode& mode) :
		_window(mode, "OBML Renderer", sf::Style::None),
		_fonts(std::make_shared<render::fonts>()),
		_path(MAX_PATH, '\0') {

		_window.setVerticalSyncEnabled(true);
		setup_imgui();

		_fonts->font.loadFromFile("C:\\Windows\\Fonts\\ARIALUNI.ttf");

#ifdef _WIN32
		setup_openfilename();
#endif
	}

	viewer::~viewer() {
		ImGui::SFML::Shutdown();
		_window.close();
	}

	void viewer::setup_imgui() {
		ImGui::SFML::Init(_window);

		ImGui::StyleColorsLight();
		ImGui::GetStyle().FrameRounding = 4.f;

		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF(
			"C:\\Windows\\Fonts\\DejaVuSansMono_0.ttf", 14.f, NULL,
			io.Fonts->GetGlyphRangesCyrillic()
		);

		ImGui::SFML::UpdateFontTexture();
	}

	void viewer::open() {
		sf::Event e;
		sf::Clock _clock;

		ImGuiContext& g = *ImGui::GetCurrentContext();
		_drawing_offset.y = std::max(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f) + g.FontBaseSize + g.Style.FramePadding.y;

		_view.setViewport({
			_drawing_offset.x, _drawing_offset.y / _window.getSize().y,
			1.f, 1.f
		});

		_view.reset({
			0.f, 0.f,
			static_cast<float>(_window.getSize().x),
			static_cast<float>(_window.getSize().y)
		});

		sf::RectangleShape _window_border({ (float)_window.getSize().x - 2, (float)_window.getSize().y - 2 });
		_window_border.setPosition(1, 1);
		_window_border.setFillColor(sf::Color::Transparent);
		_window_border.setOutlineColor({155,155,155,255});
		_window_border.setOutlineThickness(1.f);

		bool _grabbed = false;
		sf::Vector2i _grabbed_offset{ 0, 0 };

		static const sf::FloatRect _menu_bar_bounds{
			0.f, 0.f, float(_window.getSize().x), _drawing_offset.y
		};

		while (_window.isOpen()) {

			ImGui::SFML::Update(_window, _clock.restart());

			while (_window.pollEvent(e)) {
				ImGui::SFML::ProcessEvent(e);

				if (e.type == sf::Event::Closed)
					_window.close();

				else if (e.type == sf::Event::Resized) {
					const_cast<sf::View&>(_window.getDefaultView()).reset({
						0.f, 0.f,
						float(e.size.width), float(e.size.height)
					});
					_view.reset({
						_scroll.position.x, -_scroll.position.y,
						float(e.size.width), float(e.size.height)
					});

					_window_border.setSize({ float(e.size.width) - 2, float(e.size.height) - 2});
				}

				else if (e.type == sf::Event::KeyPressed) {
					switch (e.key.code) {
					case sf::Keyboard::Up:
						set_scroll_page_y(25.f);
						break;

					case sf::Keyboard::Down:
						set_scroll_page_y(-25.f);
						break;

					case sf::Keyboard::Escape:
						_window.close();
						break;
					}
				}
				else if (e.type == sf::Event::MouseWheelScrolled)
					set_scroll_page_y(e.mouseWheelScroll.delta);

				else if (e.type == sf::Event::MouseButtonReleased) {
					if (e.mouseButton.button == sf::Mouse::Button::Left && _page != nullptr) {
						if (!ImGui::GetIO().WantCaptureMouse) {
							bool found = false;

							for (auto& i : _page->get_links()) {
								if (!i.target.type.empty())
									for (const auto& j : i.regions) {
										if (j.contains((float)e.mouseButton.x, e.mouseButton.y - _scroll.position.y)) {
											_selector.show(
												{ j.left, j.top + _scroll.position.y + _drawing_offset.y },
												{ j.width, j.height },
												&i
											);
											found = true;
										}
									}
							}

							if (!found)
								_selector.hide();
						}
					}
					else if (e.mouseButton.button == sf::Mouse::Button::Right) {
						_grabbed = false;
					}
				}

				else if (e.type == sf::Event::MouseButtonPressed) {
					if (e.mouseButton.button == sf::Mouse::Button::Right) {
						if (_menu_bar_bounds.contains(e.mouseButton.x, e.mouseButton.y)) {
							_grabbed = true;
							_grabbed_offset.x = e.mouseButton.x;
							_grabbed_offset.y = e.mouseButton.y;
						}
					}
				}

				else if (e.type == sf::Event::MouseMoved) {
					if (_grabbed)
						_window.setPosition(sf::Mouse::getPosition() - _grabbed_offset);
				}
			}

			_window.clear(sf::Color::White);

			draw_main_bar();
			draw_info();
			//draw_tabs();

			if (_page != nullptr) {
				_window.setView(_view);
				_page->render(_window);
				_window.setView(_window.getDefaultView());
			}

			_window.draw(_selector);
			_window.draw(_window_border);

			ImGui::SFML::Render(_window);
			_window.display();
		}
	}

	void viewer::draw_main_bar() {
		if (ImGui::BeginMainMenuBar()) {
			ImGui::Text("OBML Renderer");

			ImGui::Separator();

			if (ImGui::MenuItem("Open")) {
#ifdef _WIN32
				if (GetOpenFileName(&_ofn) == TRUE) {
					if (_page != nullptr)
						_page.reset();

					path temp(_path);
					std::cout << "Loading page from '" << temp.stem().u8string() << "'..." << std::endl;

					_page = std::make_unique<page>(temp);

					_page->set_fonts(_fonts);
					_page->prepare();
					_page->render();

					_selector.hide();
					reset_scroll();
				}
#endif
			}

			if (ImGui::BeginMenu("Page", _page != nullptr)) {
				if (ImGui::MenuItem("Info", 0, show_page_info))
					show_page_info = !show_page_info;

				if (ImGui::BeginMenu("Save as...")) {

					if (ImGui::MenuItem("JPEG"))
						_page->export_page("", "jpeg");

					if (ImGui::MenuItem("PNG"))
						_page->export_page("", "png");

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Fonts", _page != nullptr)) {
					ImGui::SliderInt("medium", reinterpret_cast<int*>(&_fonts->font_sizes[2].size), 1, 32);
					ImGui::SliderInt("medium bold", reinterpret_cast<int*>(&_fonts->font_sizes[3].size), 1, 32);
					ImGui::SliderInt("large", reinterpret_cast<int*>(&_fonts->font_sizes[4].size), 1, 32);
					ImGui::SliderInt("large bold", reinterpret_cast<int*>(&_fonts->font_sizes[5].size), 1, 32);
					ImGui::SliderInt("small", reinterpret_cast<int*>(&_fonts->font_sizes[6].size), 1, 32);

					if (ImGui::Button("APPLY"))
						_page->update_fonts();

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Quit", ""))
				_window.close();
		}

		ImGui::EndMainMenuBar();
	}

	void viewer::draw_tabs() {
		if (_page == nullptr)
			return;

		static const ImGuiWindowFlags flags{
			ImGuiWindowFlags_HorizontalScrollbar
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoSavedSettings
		};

		/*ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TabActive] = sf::Color{ 0xD60000FF };
		style.Colors[ImGuiCol_TabUnfocused] = sf::Color{ 0x660000FF };*/

		const ImGuiContext& ctx = *ImGui::GetCurrentContext();
		float h = ctx.NextWindowData.MenuBarOffsetMinVal.y + ctx.FontBaseSize + ctx.Style.FramePadding.y; // main menu bar height

		ImGui::SetNextWindowPos({ 0.f, h }, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ float(_window.getSize().x), float(_window.getSize().y) }, ImGuiCond_Always);

		if (ImGui::Begin("#Window", 0, flags)) {
			if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
				if (ImGui::BeginTabItem(_page->get_header().title.c_str())) {
					ImGui::Image(_page->get_texture());

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("+")) {
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
	}

	void viewer::draw_info() {
		if (!show_page_info)
			return;

		const ImGuiContext& ctx = *ImGui::GetCurrentContext();
		float h = ctx.NextWindowData.MenuBarOffsetMinVal.y + ctx.FontBaseSize + ctx.Style.FramePadding.y; // main menu bar height

		static const ImGuiWindowFlags flags{
			ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoResize
//			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoSavedSettings
		};

		const float dist = 12.f;
		static int corner = 1;

		if (corner != -1) {
			ImVec2 window_pos = ImVec2((corner & 1) ? ctx.IO.DisplaySize.x - dist : dist, ((corner & 2) ? ctx.IO.DisplaySize.y - dist : dist) + h);
			ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		}

		if (ImGui::Begin("Page Info", &show_page_info, flags)) {
			const header& _header = _page->get_header();
			link* _region = _selector.get_region();

			ImGui::Text("HEADER");
			ImGui::Separator();
			ImGui::BulletText("Size: %dKB", _header.data_len / 1024);
			ImGui::BulletText("Version: %d", _header.version);
			ImGui::BulletText("Resolution: %dx%d", _header.size.x, _header.size.y);
			ImGui::Bullet();
			ImGui::TextWrapped("Title: %s", _header.title.c_str());

			ImGui::BulletText("URL");
			ImGui::SameLine();
			ImGui::InputText("",
				const_cast<char*>(_header.base_url.c_str()),
				_header.base_url.size() + 1,
				ImGuiInputTextFlags_AutoSelectAll
				| ImGuiInputTextFlags_ReadOnly
			);
			ImGui::SameLine();
			if (ImGui::Button("COPY")) {
				ImGui::LogToClipboard();
				ImGui::LogText(_header.base_url.c_str());
				ImGui::LogFinish();
			}

			ImGui::Text(" ");
			ImGui::Text("SELECTED REGION");
			ImGui::Separator();
			if (_region != nullptr) {
				ImGui::BulletText(
					"Dimension: %.0fx%.0f, %.0fx%.0f",
					_region->regions.front().top, _region->regions.front().left,
					_region->regions.front().width, _region->regions.front().height
				);
				ImGui::BulletText("Type: %s", _region->target.type.c_str());

				ImGui::BulletText("Target:"); ImGui::SameLine();
				ImGui::InputText("##target",
					_region->target.href.data(),
					_region->target.href.size() + 1,
					ImGuiInputTextFlags_AutoSelectAll
					| ImGuiInputTextFlags_ReadOnly
				);

				if (ImGui::Button("EXPORT"))
					_page->export_region("", _region->regions.front());
			}
			else
				ImGui::Text("EMPTY");
		}
		ImGui::End();
	}

	void viewer::set_scroll_page_y(float amount, float factor) {
		if (_page == nullptr)
			return;

		sf::Vector2f size = _view.getSize();
		float page_height = _page->get_header().size.y;

		float step = amount * factor;
		float max_step = -(page_height - size.y);

		float after_step = _scroll.position.y + step;

		if (amount > 0 && after_step > 0.f)
			if (_scroll.position.y < 0.f)
				step = -_scroll.position.y;
			else
				step = 0.f;

		else if (after_step < max_step)
			if (page_height > size.y)
				step = max_step - _scroll.position.y - _drawing_offset.y;
			else
				step = 0.f;

		_scroll.position.y += step;

		_view.reset({
			_scroll.position.x, -_scroll.position.y,
			size.x, size.y
		});

		_selector.move(0, step);
	}

	void viewer::reset_scroll() {
		sf::Vector2f size = _view.getSize();
		_scroll.position = { 0, 0 };

		_view.reset({
			_scroll.position.x, _scroll.position.y,
			size.x, size.y
		});
	}

#ifdef _WIN32
	void viewer::setup_openfilename() {
		ZeroMemory(&_ofn, sizeof _ofn);

		_ofn.lStructSize = sizeof _ofn;
		_ofn.hwndOwner = _window.getSystemHandle();
		_ofn.lpstrFilter = L"Opera Binary Markup Language\0*.obml";
		_ofn.lpstrTitle = L"Select OBML file...";
		_ofn.lpstrFile = &_path[0];
		_ofn.nMaxFile = MAX_PATH;
		_ofn.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_READONLY;
	}
#endif

	const sf::Color selector::outline(55, 64, 164, 255);
	const sf::Color selector::fill(55, 64, 164, 80);

	selector::selector() {
		rect.setFillColor(fill);
		rect.setOutlineColor(outline);
		rect.setOutlineThickness(2.f);
	}

	void selector::draw(sf::RenderTarget& target, sf::RenderStates states) const {
		if (has_show)
			target.draw(rect);
	}

	void selector::show(const sf::Vector2f& position, const sf::Vector2f& size, link* region) {
		has_show = true;

		rect.setPosition(position);
		rect.setSize(size);

		this->region = region;
	}

	void selector::move(float x, float y) {
		rect.move(x, y);
	}

	void selector::hide() {
		has_show = false;
		region = nullptr;
	}

	link* selector::get_region() {
		return region;
	}
};