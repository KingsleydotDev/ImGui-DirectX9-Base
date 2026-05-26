#pragma once

#include <Windows.h>
#include <string>

#include "../../ext/imgui/imgui.h"
#include "pages/pages.hpp"

namespace gui
{
	inline std::string MenuTitle = "Corrupted by Kingsley";
	inline bool open = true;

	inline constexpr int WIDTH = 790;
	inline constexpr int HEIGHT = 578;

	void UpdateMenuTitle();
	void Render() noexcept;
}
