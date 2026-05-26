#include "gui.hpp"

#include <cstdlib>

#include "pages/misc/misc.hpp"
#include "pages/visuals/visuals.hpp"

namespace gui
{
	void UpdateMenuTitle()
	{
		static DWORD lastUpdate = 0;
		static DWORD cycleStart = GetTickCount();
		const DWORD currentTime = GetTickCount();

		if (currentTime - lastUpdate > 50)
		{
			lastUpdate = currentTime;
			const DWORD timeInCycle = (currentTime - cycleStart) % 1000;

			if (timeInCycle > 500)
			{
				std::string randomStr;
				const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
				for (size_t i = 0; i < 9; ++i)
				{
					randomStr += charset[rand() % (sizeof(charset) - 1)];
				}
				MenuTitle = randomStr + " by Kingsley";
			}
			else if (MenuTitle != "Corrupted by Kingsley")
			{
				MenuTitle = "Corrupted by Kingsley";
			}
		}
	}

	void Render() noexcept
	{
		const std::string dynamicTitle = MenuTitle + "###MenuID";

		const ImVec4 accent = ImVec4(
			vars::colorAccent[0],
			vars::colorAccent[1],
			vars::colorAccent[2],
			vars::colorAccent[3]);
		const ImVec4 accentHover = ImVec4(
			accent.x + 0.12f > 1.0f ? 1.0f : accent.x + 0.12f,
			accent.y + 0.12f > 1.0f ? 1.0f : accent.y + 0.12f,
			accent.z + 0.12f > 1.0f ? 1.0f : accent.z + 0.12f,
			accent.w);
		const ImVec4 accentActive = ImVec4(
			accent.x * 0.80f,
			accent.y * 0.80f,
			accent.z * 0.80f,
			accent.w);

		ImGui::PushStyleColor(ImGuiCol_CheckMark, accent);
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent);
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accentHover);
		ImGui::PushStyleColor(ImGuiCol_Button, accent);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, accentHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, accentActive);
		ImGui::PushStyleColor(ImGuiCol_Header, accent);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, accentHover);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, accentActive);
		ImGui::PushStyleColor(ImGuiCol_Tab, accentActive);
		ImGui::PushStyleColor(ImGuiCol_TabHovered, accentHover);
		ImGui::PushStyleColor(ImGuiCol_TabActive, accent);

		ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_FirstUseEver);

		if (ImGui::Begin(dynamicTitle.c_str(), &open))
		{
			if (ImGui::BeginTabBar("##MainTabs"))
			{
				if (ImGui::BeginTabItem("Player"))
				{
					pages::RenderPlayer();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Visuals"))
				{
					pages::RenderVisuals();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Misc"))
				{
					pages::RenderMisc();
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}

		ImGui::End();
		ImGui::PopStyleColor(12);
	}
}
