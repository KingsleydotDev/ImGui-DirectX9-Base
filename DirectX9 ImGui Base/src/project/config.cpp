#include "config.hpp"

#include <Windows.h>

#include <cstdlib>
#include <string>

#include "../../ext/mINI/ini.h"
#include "../../ext/imgui/imgui.h"

#include "../gui/pages/misc/misc.hpp"
#include "../gui/pages/player/player.hpp"
#include "../gui/pages/visuals/visuals.hpp"

namespace
{
	bool ParseBool(const std::string& value, const bool defaultValue) noexcept
	{
		if (value.empty())
		{
			return defaultValue;
		}

		return value == "1" || value == "true" || value == "True" || value == "TRUE";
	}

	int ParseInt(const std::string& value, const int defaultValue) noexcept
	{
		if (value.empty())
		{
			return defaultValue;
		}

		try
		{
			return std::stoi(value);
		}
		catch (...)
		{
			return defaultValue;
		}
	}

	float ParseFloat(const std::string& value, const float defaultValue) noexcept
	{
		if (value.empty())
		{
			return defaultValue;
		}

		try
		{
			return std::stof(value);
		}
		catch (...)
		{
			return defaultValue;
		}
	}

	std::string GetValue(
		const mINI::INIStructure& ini,
		const std::string& section,
		const std::string& key) noexcept
	{
		return ini.get(section).get(key);
	}
}

std::string config::GetConfigPath()
{
	char modulePath[MAX_PATH]{};
	const DWORD length = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
	if (length == 0 || length >= MAX_PATH)
	{
		return FILE_NAME;
	}

	std::string path(modulePath, modulePath + length);
	const size_t slash = path.find_last_of("\\/");
	if (slash != std::string::npos)
	{
		path.resize(slash + 1);
	}
	else
	{
		path.clear();
	}

	path += FILE_NAME;
	return path;
}

bool config::Load()
{
	const std::string path = GetConfigPath();
	mINI::INIFile file(path);
	mINI::INIStructure ini;

	if (!file.read(ini))
	{
		return false;
	}

	vars::bGodMode = ParseBool(GetValue(ini, "Player", "GodMode"), vars::bGodMode);
	vars::bNoClip = ParseBool(GetValue(ini, "Player", "NoClip"), vars::bNoClip);
	vars::iFPS = ParseInt(GetValue(ini, "Player", "FPS"), vars::iFPS);

	vars::fFOV = ParseFloat(GetValue(ini, "Visuals", "FOV"), vars::fFOV);
	vars::bESP = ParseBool(GetValue(ini, "Visuals", "ESP"), vars::bESP);
	vars::bCrosshair = ParseBool(GetValue(ini, "Visuals", "Crosshair"), vars::bCrosshair);

	vars::iTheme = ParseInt(GetValue(ini, "Misc", "Theme"), vars::iTheme);
	vars::colorAccent[0] = ParseFloat(GetValue(ini, "Misc", "AccentR"), vars::colorAccent[0]);
	vars::colorAccent[1] = ParseFloat(GetValue(ini, "Misc", "AccentG"), vars::colorAccent[1]);
	vars::colorAccent[2] = ParseFloat(GetValue(ini, "Misc", "AccentB"), vars::colorAccent[2]);
	vars::colorAccent[3] = ParseFloat(GetValue(ini, "Misc", "AccentA"), vars::colorAccent[3]);

	if (ImGui::GetCurrentContext() != nullptr)
	{
		gui::pages::ApplyTheme(vars::iTheme);
	}

	return true;
}

bool config::Save()
{
	mINI::INIStructure ini;

	ini["Player"]["GodMode"] = vars::bGodMode ? "1" : "0";
	ini["Player"]["NoClip"] = vars::bNoClip ? "1" : "0";
	ini["Player"]["FPS"] = std::to_string(vars::iFPS);

	ini["Visuals"]["FOV"] = std::to_string(vars::fFOV);
	ini["Visuals"]["ESP"] = vars::bESP ? "1" : "0";
	ini["Visuals"]["Crosshair"] = vars::bCrosshair ? "1" : "0";

	ini["Misc"]["Theme"] = std::to_string(vars::iTheme);
	ini["Misc"]["AccentR"] = std::to_string(vars::colorAccent[0]);
	ini["Misc"]["AccentG"] = std::to_string(vars::colorAccent[1]);
	ini["Misc"]["AccentB"] = std::to_string(vars::colorAccent[2]);
	ini["Misc"]["AccentA"] = std::to_string(vars::colorAccent[3]);

	mINI::INIFile file(GetConfigPath());
	return file.write(ini);
}

void config::TryLoadOnStartup() noexcept
{
	const DWORD attributes = GetFileAttributesA(GetConfigPath().c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	(void)Load();
}
