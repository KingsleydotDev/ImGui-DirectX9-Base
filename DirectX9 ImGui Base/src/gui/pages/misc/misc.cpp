#include "../pages.hpp"
#include "misc.hpp"

namespace gui::pages
{
	void ApplyTheme(const int theme) noexcept
	{
		switch (theme)
		{
		case 1:
			ImGui::StyleColorsClassic();
			break;
		case 2:
			ImGui::StyleColorsLight();
			break;
		default:
			ImGui::StyleColorsDark();
			break;
		}
	}

	void RenderMisc() noexcept
	{
		const char* themes[] = { "Dark", "Classic", "Light" };
		if (ImGui::Combo("Theme", &vars::iTheme, themes, IM_ARRAYSIZE(themes)))
		{
			ApplyTheme(vars::iTheme);
		}

		ImGui::ColorEdit4("Accent Color", vars::colorAccent);
	}
}
