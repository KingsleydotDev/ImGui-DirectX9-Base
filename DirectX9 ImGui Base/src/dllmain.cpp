#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>
#include <cstdint>
#include <cstdio>

#include "gui/bootstrapper.hpp"
#include "gui/gui.hpp"
#include "game/hooks.hpp"
#include "project/config.hpp"

void MainThread(const HMODULE instance)
{
	try
	{
		FILE* fp = nullptr;
		freopen_s(&fp, "CONOUT$", "w", stdout);

		Setup();
		game::Setup();
		config::TryLoadOnStartup();
	}
	catch (const std::exception& error)
	{
		MessageBeep(MB_ICONERROR);
		MessageBoxA(0, error.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
		goto UNLOAD;
	}

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		gui::UpdateMenuTitle();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

UNLOAD:
	game::Destroy();
	Destroy();
	if (stdout)
	{
		fclose(stdout);
	}

	FreeLibraryAndExitThread(instance, 0);
}

BOOL WINAPI DllMain(const HMODULE instance, const std::uintptr_t reason, const void* reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);
		const auto thread = CreateThread(
			nullptr,
			0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread),
			instance,
			0,
			nullptr);
		if (thread)
		{
			CloseHandle(thread);
		}
	}

	return TRUE;
}
