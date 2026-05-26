#include "../pages.hpp"
#include "config.hpp"

#include "../../../project/config.hpp"
#include "../misc/misc.hpp"

namespace gui::pages
{
	void RenderConfig() noexcept
	{
		ImGui::TextWrapped("Config file: %s", config::GetConfigPath().c_str());
		ImGui::Spacing();

		static const char* statusMessage = "";
		static bool statusSuccess = true;

		if (ImGui::Button("Save", ImVec2(120.0f, 0.0f)))
		{
			if (config::Save())
			{
				statusMessage = "Config saved.";
				statusSuccess = true;
			}
			else
			{
				statusMessage = "Failed to save config.";
				statusSuccess = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Load", ImVec2(120.0f, 0.0f)))
		{
			if (config::Load())
			{
				statusMessage = "Config loaded.";
				statusSuccess = true;
			}
			else
			{
				statusMessage = "Failed to load config.";
				statusSuccess = false;
			}
		}

		if (statusMessage[0] != '\0')
		{
			ImGui::Spacing();
			if (statusSuccess)
			{
				ImGui::TextUnformatted(statusMessage);
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", statusMessage);
			}
		}
	}
}
