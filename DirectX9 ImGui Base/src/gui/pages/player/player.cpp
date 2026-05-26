#include "../pages.hpp"
#include "player.hpp"

namespace gui::pages
{
	void RenderPlayer() noexcept
	{
		ImGui::Checkbox("God Mode", &vars::bGodMode);
		ImGui::Checkbox("No Clip", &vars::bNoClip);
		ImGui::SliderInt("FPS", &vars::iFPS, 30, 300);
	}
}
