#pragma once

namespace vars
{
	inline int iTheme = 0;
	inline float colorAccent[4] = { 0.20f, 0.55f, 0.95f, 1.00f };
}

namespace gui::pages
{
	void ApplyTheme(int theme) noexcept;
}
