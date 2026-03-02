#pragma once
#include <d3d9.h>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx9.h"
#include "../ext/imgui/imgui_internal.h"

#include "functions.hpp"
#include "vars.hpp"

namespace gui {
	inline std::string MenuTitle = "Corrupted by Kingsley";

	inline void UpdateMenuTitle() {
		static DWORD lastUpdate = 0;
		static DWORD cycleStart = GetTickCount();
		DWORD currentTime = GetTickCount();

		if (currentTime - lastUpdate > 50) {
			lastUpdate = currentTime;
			DWORD timeInCycle = (currentTime - cycleStart) % 1000;

			if (timeInCycle > 500) {
				std::string randomStr;
				const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
				for (size_t i = 0; i < 9; ++i) {
					randomStr += charset[rand() % (sizeof(charset) - 1)];
				}
				MenuTitle = randomStr + " by Kingsley";
			}
			else if (MenuTitle != "Corrupted by Kingsley") {
				MenuTitle = "Corrupted by Kingsley";
			}
		}
	}
}

// menu size
#define WIDTH 790
#define HEIGHT 578



namespace gui
{
	// Show menu?
	inline bool open = true;

	// is it setup?
	inline bool setup = false;


	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = {};
	inline WNDPROC originalWindowProcess = nullptr;

	//dx 
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline LPDIRECT3D9 d3d9 = nullptr;

	inline bool SetupWindowClass(const char* windowClassName) noexcept;
	inline void DestroyWindowClass() noexcept;

	inline bool SetupWindow(const char* windowName) noexcept;
	inline void DestroyWindow() noexcept;

	inline bool SetupDirectX() noexcept;
	inline void DestroyDirectX() noexcept;

	// Setup Device
	inline void Setup();

	inline void SetupMenu(LPDIRECT3DDEVICE9 device) noexcept;
	inline void Destroy() noexcept;

	inline void Render() noexcept;
	//void RenderESP() noexcept;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

inline LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

inline bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = NULL;

	if (!RegisterClassEx(&windowClass))
	{
		return false;
	}

	return true;
}

inline void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		windowClass.lpszClassName,
		windowClass.hInstance
	);
}

inline bool gui::SetupWindow(const char* windowName) noexcept
{
	window = CreateWindow(
		windowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		0,
		0,
		windowClass.hInstance,
		0
	);
	if (!window) {
		return false;
	}
	return true;
}

inline void gui::DestroyWindow() noexcept
{
	if (window) {
		DestroyWindow(window);
	}
}

inline bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
	{
		return false;
	}

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(handle, "Direct3DCreate9"));

	if (!create)
	{
		return false;
	}

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
	{
		return false;
	}

	D3DPRESENT_PARAMETERS params = { };
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&params,
		&device) < 0) return false;

	return true;
}

inline void gui::DestroyDirectX() noexcept
{
	if (device)
	{
		device->Release();
		device = NULL;
	}
	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}

inline void gui::Setup()
{
	if (!SetupWindowClass("hackClass001"))
		throw std::runtime_error("Failed to create window class.");

	if (!SetupWindow("Hack Window"))
		throw std::runtime_error("Failed to create window.");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to setup DirectX.");

	DestroyWindow();
	DestroyWindowClass();
}

inline void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	auto params = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&params);

	window = params.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
	setup = true;
}

inline void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	DestroyDirectX();
}

inline void gui::Render() noexcept
{
	//title
	std::string dynamicTitle = MenuTitle + "###MenuID";

	//accent colour usage.
	const ImVec4 accent = ImVec4(vars::colorAccent[0], vars::colorAccent[1], vars::colorAccent[2], vars::colorAccent[3]);
	const ImVec4 accentHover = ImVec4(accent.x + 0.12f > 1.0f ? 1.0f : accent.x + 0.12f,
		accent.y + 0.12f > 1.0f ? 1.0f : accent.y + 0.12f,
		accent.z + 0.12f > 1.0f ? 1.0f : accent.z + 0.12f,
		accent.w);
	const ImVec4 accentActive = ImVec4(accent.x * 0.80f, accent.y * 0.80f, accent.z * 0.80f, accent.w);

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

	// window size 
	ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_FirstUseEver);

	//menu
	if (ImGui::Begin(dynamicTitle.c_str(), &open))
	{
		if (ImGui::BeginTabBar("##MainTabs"))
		{
			if (ImGui::BeginTabItem("Player"))
			{
				ImGui::Checkbox("God Mode", &vars::bGodMode);
				ImGui::Checkbox("No Clip", &vars::bNoClip);
				ImGui::SliderInt("FPS", &vars::iFPS, 30, 300);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Visuals"))
			{
				ImGui::SliderFloat("FOV", &vars::fFOV, 60.0f, 140.0f, "%.1f");
				ImGui::Checkbox("ESP", &vars::bESP);
				ImGui::Checkbox("Crosshair", &vars::bCrosshair);
				ImGui::ColorEdit4("Accent Color", vars::colorAccent);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{
				const char* themes[] = { "Dark", "Classic", "Light" };
				ImGui::Combo("Theme", &vars::iTheme, themes, IM_ARRAYSIZE(themes));

				switch (vars::iTheme)
				{
				case 1: ImGui::StyleColorsClassic(); break;
				case 2: ImGui::StyleColorsLight(); break;
				default: ImGui::StyleColorsDark(); break;
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor(12);
}

inline LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
)
{
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		gui::open = !gui::open;
	}
	if (gui::open)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = true;
		// game mouse input
		//*reinterpret_cast<int**>(0x6427D3D) = nullptr;
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = false;
		// game mouse input
		//*reinterpret_cast<int*>(0x6427D3D) = 1;
	}

	if (gui::open && ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wideParam,
		longParam
	)) return 1L;

	return CallWindowProc(
		gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}