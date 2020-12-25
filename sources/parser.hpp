#pragma once

#include "main.hpp"

namespace obml_renderer {
	class parser : private sf::NonCopyable {
	public:
		enum ver {
			v6 = 6
		};

		enum err {
			none,
			version,
			bad_link_tag,
			bad_content_tag,
			bad_data,
			bad_path,
			unknown
		};

		explicit parser(page& p);
		~parser();

		err parser::parse();

	private:
		reader _reader;
		page& _page;

		sf::Int64 _links_begin;
		sf::Int64 _links_end;
		sf::Int64 _links_size;

		err read_header();
		err read_metadata();
		err read_links();
		err read_content();
	};
};