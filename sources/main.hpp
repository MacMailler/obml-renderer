#pragma once

#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <variant>
#include <filesystem>
#include <unordered_map>

#include <SFML\Graphics.hpp>

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui-SFML.h"

#include "entity.hpp"
#include "page.hpp"
#include "reader.hpp"
#include "parser.hpp"
#include "viewer.hpp"

#define __Debug__
//#define __DebugVerbose__
//#define __NoConsole__