#pragma once

#include <string>

namespace config
{
	inline constexpr const char* FILE_NAME = "Corrupted.ini";

	std::string GetConfigPath();
	bool Load();
	bool Save();
	void TryLoadOnStartup() noexcept;
}
