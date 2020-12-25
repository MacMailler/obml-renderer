#include "viewer.hpp"

namespace obml_renderer {
	viewer::viewer(const sf::VideoMode& mode) :
		_window(mode, "OBML Renderer", sf::Style::None),
		_fonts(std::make_shared<render::fonts>()),
		_path(MAX_PATH, '\0') {

		_window.setVerticalSyncEnabled(true);
		setup_imgui();

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

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		style.FrameRounding = 4.f;

		io.FontDefault = io.Fonts->AddFontFromFileTTF(
			"C:\\Windows\\Fonts\\DejaVuSansMono_0.ttf", 14.6f, NULL,
			io.Fonts->GetGlyphRangesCyrillic()
		);

		ImGui::SFML::UpdateFontTexture();
		ImGui::StyleColorsLight();
	}

	void viewer::open() {
		_fonts->font.loadFromFile("C:\\Windows\\Fonts\\ARIALUNI.ttf");

		_view.reset({
			0.f, 0.f,
			static_cast<float>(_window.getSize().x),
			static_cast<float>(_window.getSize().y)
		});

		sf::Event e;
		sf::Clock _clock;
		while (_window.isOpen()) {
			ImGui::SFML::ProcessEvent(e);
			while (_window.pollEvent(e)) {
				if (e.type == sf::Event::Closed)
					_window.close();

				else if (e.type == sf::Event::Resized)
					_view.reset({
						_scroll.position.x, -_scroll.position.y,
						float(e.size.width), float(e.size.height)
					});

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
												{ j.left, j.top + _scroll.position.y },
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
				}
			}

			_window.clear(sf::Color::White);

			ImGui::SFML::Update(_window, _clock.restart());

			draw_main_bar();
			draw_info();
			//draw_tabs();

			if (_page != nullptr) {
				_window.setView(_view);
				_page->render(_window);
				_window.setView(_window.getDefaultView());
			}

			_window.draw(_selector);

			ImGui::SFML::Render(_window);
			_window.display();
		}
	}

	void viewer::draw_main_bar() {
		if (ImGui::BeginMainMenuBar()) {
			/*if (ImGui::BeginMenu("OBML Renderer 0.1")) {
				ImGui::EndMenu();
			}*/

			ImGui::Text("OBML Renderer");

			ImGui::Separator();

			if (ImGui::MenuItem("Open")) {
#ifdef _WIN32
				if (GetOpenFileNameW(&_ofn) == TRUE) {
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
					ImGui::InputInt("medium", reinterpret_cast<int*>(&_fonts->font_sizes[2].size));
					ImGui::InputInt("medium bold", reinterpret_cast<int*>(&_fonts->font_sizes[3].size));
					ImGui::InputInt("large", reinterpret_cast<int*>(&_fonts->font_sizes[4].size));
					ImGui::InputInt("large bold", reinterpret_cast<int*>(&_fonts->font_sizes[5].size));
					ImGui::InputInt("small", reinterpret_cast<int*>(&_fonts->font_sizes[6].size));

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
		sf::Vector2f size = _view.getSize();
		float step = amount * factor;
		float max_step = -(_page->get_header().size.y - size.y);

		float after_step = _scroll.position.y + step;

		if (amount > 0 && after_step > 0.f)
			if (_scroll.position.y < 0)
				step = -_scroll.position.y;
			else
				step = 0.f;

		else if (after_step < max_step)
			if (size.y > _window.getSize().y)
				step = max_step - _scroll.position.y;
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