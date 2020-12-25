#include "main.hpp"

using namespace obml_renderer;


static const uint32_t width = 1280;
static const uint32_t height = width / 16*9;


#if defined __NoConsole__
	#if defined _WIN32
		int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
			_In_opt_ HINSTANCE hPrevInstance,
			_In_ LPWSTR    lpCmdLine,
			_In_ int       nCmdShow)
	#else
		int main()
	#endif
#else
	int main()
#endif
{
#if defined _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	setvbuf(stdout, nullptr, _IOFBF, 1024);

#ifdef __NoConsole__
	std::fstream log("log.txt", std::ios::trunc);
	std::cout.set_rdbuf(log.rdbuf());
	sf::err().set_rdbuf(log.rdbuf());
#endif

	viewer _viewer({ width, height });
	_viewer.open();

	return 0;
}