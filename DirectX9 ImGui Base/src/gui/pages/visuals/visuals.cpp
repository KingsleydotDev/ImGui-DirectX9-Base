#include "../pages.hpp"
#include "visuals.hpp"

namespace gui::pages
{
	void RenderVisuals() noexcept
	{
		ImGui::SliderFloat("FOV", &vars::fFOV, 60.0f, 140.0f, "%.1f");
		ImGui::Checkbox("ESP", &vars::bESP);
		ImGui::Checkbox("Crosshair", &vars::bCrosshair);
	}
}
