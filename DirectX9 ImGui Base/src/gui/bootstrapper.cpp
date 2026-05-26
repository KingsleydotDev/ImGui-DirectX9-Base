#include "bootstrapper.hpp"
#include "gui.hpp"
#include "pages/misc/misc.hpp"

#include <cstdio>
#include <stdexcept>
#include <intrin.h>

#include "../../ext/minhook/minhook.h"
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam);

namespace
{
	struct ProcessWindowSearch
	{
		DWORD processId = 0;
		HWND bestWindow = nullptr;
		LONG bestArea = 0;
	};

	BOOL CALLBACK CollectProcessWindow(HWND hwnd, LPARAM param) noexcept
	{
		auto* search = reinterpret_cast<ProcessWindowSearch*>(param);
		DWORD windowProcessId = 0;
		GetWindowThreadProcessId(hwnd, &windowProcessId);
		if (windowProcessId != search->processId)
		{
			return TRUE;
		}

		if (!IsWindowVisible(hwnd))
		{
			return TRUE;
		}

		RECT clientRect{};
		if (!GetClientRect(hwnd, &clientRect))
		{
			return TRUE;
		}

		const LONG width = clientRect.right - clientRect.left;
		const LONG height = clientRect.bottom - clientRect.top;
		if (width <= 0 || height <= 0)
		{
			return TRUE;
		}

		const LONG area = width * height;
		if (area > search->bestArea)
		{
			search->bestArea = area;
			search->bestWindow = hwnd;
		}

		return TRUE;
	}

	HWND FindLargestProcessWindow() noexcept
	{
		ProcessWindowSearch search{};
		search.processId = GetCurrentProcessId();
		EnumWindows(CollectProcessWindow, reinterpret_cast<LPARAM>(&search));
		return search.bestWindow;
	}

	bool WaitForD3D9(const int maxAttempts, const DWORD sleepMs) noexcept
	{
		for (int attempt = 0; attempt < maxAttempts; ++attempt)
		{
			const HMODULE module = GetModuleHandleA("d3d9.dll");
			if (module != nullptr && GetProcAddress(module, "Direct3DCreate9") != nullptr)
			{
				return true;
			}

			Sleep(sleepMs);
		}

		return false;
	}

	bool CreateProbeDevice(IDirect3DDevice9** outDevice) noexcept
	{
		if (outDevice == nullptr)
		{
			return false;
		}

		*outDevice = nullptr;

		const HMODULE module = GetModuleHandleA("d3d9.dll");
		if (module == nullptr)
		{
			return false;
		}

		using CreateFn = IDirect3D9*(WINAPI*)(UINT);
		const auto create = reinterpret_cast<CreateFn>(GetProcAddress(module, "Direct3DCreate9"));
		if (create == nullptr)
		{
			return false;
		}

		IDirect3D9* d3d9 = create(D3D_SDK_VERSION);
		if (d3d9 == nullptr)
		{
			return false;
		}

		WNDCLASSEXA windowClass{};
		windowClass.cbSize = sizeof(WNDCLASSEXA);
		windowClass.lpfnWndProc = DefWindowProcA;
		windowClass.hInstance = GetModuleHandleA(nullptr);
		windowClass.lpszClassName = "dx9_probe_class";

		if (!RegisterClassExA(&windowClass))
		{
			d3d9->Release();
			return false;
		}

		HWND probeWindow = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"dx9_probe",
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			100,
			100,
			nullptr,
			nullptr,
			windowClass.hInstance,
			nullptr);

		if (probeWindow == nullptr)
		{
			UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
			d3d9->Release();
			return false;
		}

		D3DPRESENT_PARAMETERS params{};
		params.BackBufferWidth = 0;
		params.BackBufferHeight = 0;
		params.BackBufferFormat = D3DFMT_UNKNOWN;
		params.BackBufferCount = 0;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;
		params.MultiSampleQuality = 0;
		params.hDeviceWindow = probeWindow;
		params.Windowed = TRUE;
		params.EnableAutoDepthStencil = FALSE;
		params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
		params.Flags = 0;
		params.FullScreen_RefreshRateInHz = 0;
		params.PresentationInterval = 0;

		IDirect3DDevice9* probeDevice = nullptr;
		const HRESULT createResult = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_NULLREF,
			probeWindow,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
			&params,
			&probeDevice);

		d3d9->Release();
		DestroyWindow(probeWindow);
		UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);

		if (FAILED(createResult) || probeDevice == nullptr)
		{
			return false;
		}

		*outDevice = probeDevice;
		return true;
	}

	void ReleaseProbeDevice(IDirect3DDevice9* probeDevice) noexcept
	{
		if (probeDevice != nullptr)
		{
			probeDevice->Release();
		}
	}

	bool ShouldBlockGameKeys(const UINT message) noexcept
	{
		return message >= WM_KEYFIRST && message <= WM_KEYLAST;
	}
}

void OnBeforeReset() noexcept
{
	if (shuttingDown || !isSetup)
	{
		return;
	}

	ImGui_ImplDX9_InvalidateDeviceObjects();
}

bool TryReattachWindow(const HWND newWindow) noexcept
{
	if (newWindow == nullptr)
	{
		return false;
	}

	if (newWindow == window)
	{
		return true;
	}

	if (window != nullptr && originalWindowProcess != nullptr)
	{
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));
	}

	ImGui_ImplWin32_Shutdown();

	window = newWindow;
	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	return ImGui_ImplWin32_Init(window);
}

bool TryRefreshDevice(IDirect3DDevice9* newDevice) noexcept
{
	if (newDevice == nullptr)
	{
		return false;
	}

	if (newDevice == device)
	{
		return true;
	}

	ImGui_ImplDX9_Shutdown();

	if (device != nullptr)
	{
		device->Release();
		device = nullptr;
	}

	device = newDevice;
	device->AddRef();
	ImGui_ImplDX9_Init(device);

	return true;
}

bool EnsureDeviceBinding(IDirect3DDevice9* currentDevice) noexcept
{
	if (!isSetup || currentDevice == nullptr)
	{
		return false;
	}

	const HRESULT cooperativeLevel = currentDevice->TestCooperativeLevel();
	if (cooperativeLevel != D3D_OK)
	{
		return false;
	}

	if (!TryRefreshDevice(currentDevice))
	{
		return false;
	}

	D3DDEVICE_CREATION_PARAMETERS creationParams{};
	if (FAILED(currentDevice->GetCreationParameters(&creationParams)))
	{
		return false;
	}

	if (!TryReattachWindow(creationParams.hFocusWindow))
	{
		return false;
	}

	D3DVIEWPORT9 viewport{};
	if (FAILED(currentDevice->GetViewport(&viewport)))
	{
		return false;
	}

	const UINT width = viewport.Width;
	const UINT height = viewport.Height;
	const bool bufferSizeChanged =
		width != backBufferWidth ||
		height != backBufferHeight;

	if (bufferSizeChanged)
	{
		backBufferWidth = width;
		backBufferHeight = height;
		ImGui_ImplDX9_InvalidateDeviceObjects();
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return true;
}

void AbortOverlaySetup() noexcept
{
	if (ImGui::GetCurrentContext() != nullptr)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	if (window != nullptr && originalWindowProcess != nullptr)
	{
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));
	}

	window = nullptr;
	originalWindowProcess = nullptr;
	backBufferWidth = 0;
	backBufferHeight = 0;

	if (device != nullptr)
	{
		device->Release();
		device = nullptr;
	}

	isSetup = false;
}

void SetupOverlay(IDirect3DDevice9* currentDevice) noexcept
{
	if (currentDevice == nullptr || isSetup)
	{
		return;
	}

	D3DDEVICE_CREATION_PARAMETERS creationParams{};
	if (FAILED(currentDevice->GetCreationParameters(&creationParams)))
	{
		return;
	}

	window = creationParams.hFocusWindow;
	if (window == nullptr)
	{
		return;
	}

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	device = currentDevice;
	device->AddRef();

	D3DVIEWPORT9 viewport{};
	if (SUCCEEDED(currentDevice->GetViewport(&viewport)))
	{
		backBufferWidth = viewport.Width;
		backBufferHeight = viewport.Height;
	}

	ImGui::CreateContext();
	gui::pages::ApplyTheme(vars::iTheme);

	if (!ImGui_ImplWin32_Init(window))
	{
		AbortOverlaySetup();
		return;
	}

	if (!ImGui_ImplDX9_Init(device))
	{
		AbortOverlaySetup();
		return;
	}

	isSetup = true;
}

void TeardownOverlay() noexcept
{
	if (!isSetup && ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (window != nullptr && originalWindowProcess != nullptr)
	{
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));
	}

	window = nullptr;
	originalWindowProcess = nullptr;
	backBufferWidth = 0;
	backBufferHeight = 0;

	if (device != nullptr)
	{
		device->Release();
		device = nullptr;
	}

	isSetup = false;
}

void Setup()
{
	if (MH_Initialize() != MH_OK)
	{
		throw std::runtime_error("Unable to initialize minhook");
	}

	constexpr int maxAttempts = 50;
	if (!WaitForD3D9(maxAttempts, 100))
	{
		throw std::runtime_error("Timed out waiting for d3d9.dll");
	}

	(void)FindLargestProcessWindow();

	IDirect3DDevice9* probeDevice = nullptr;
	bool probeCreated = false;

	for (int attempt = 0; attempt < maxAttempts; ++attempt)
	{
		if (CreateProbeDevice(&probeDevice))
		{
			probeCreated = true;
			break;
		}

		Sleep(100);
	}

	if (!probeCreated || probeDevice == nullptr)
	{
		throw std::runtime_error("Failed to create D3D9 probe device");
	}

	if (MH_CreateHook(
		VirtualFunction(probeDevice, 42),
		&EndScene,
		reinterpret_cast<void**>(&EndSceneOriginal)) != MH_OK)
	{
		ReleaseProbeDevice(probeDevice);
		throw std::runtime_error("Unable to hook EndScene()");
	}

	if (MH_CreateHook(
		VirtualFunction(probeDevice, 16),
		&Reset,
		reinterpret_cast<void**>(&ResetOriginal)) != MH_OK)
	{
		ReleaseProbeDevice(probeDevice);
		throw std::runtime_error("Unable to hook Reset()");
	}

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		ReleaseProbeDevice(probeDevice);
		throw std::runtime_error("Unable to enable hooks");
	}

	ReleaseProbeDevice(probeDevice);
}

void Destroy() noexcept
{
	if (shuttingDown)
	{
		return;
	}

	shuttingDown = true;

	MH_DisableHook(MH_ALL_HOOKS);
	Sleep(150);

	TeardownOverlay();

	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}

HRESULT __stdcall Reset(
	IDirect3DDevice9* currentDevice,
	D3DPRESENT_PARAMETERS* params) noexcept
{
	if (shuttingDown)
	{
		return ResetOriginal(currentDevice, params);
	}

	OnBeforeReset();

	const HRESULT result = ResetOriginal(currentDevice, params);

	if (isSetup && SUCCEEDED(result))
	{
		if (params != nullptr)
		{
			backBufferWidth = params->BackBufferWidth;
			backBufferHeight = params->BackBufferHeight;
		}

		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return result;
}

long __stdcall EndScene(IDirect3DDevice9* currentDevice) noexcept
{
	if (shuttingDown)
	{
		return EndSceneOriginal(currentDevice);
	}

	static const auto returnAddress = _ReturnAddress();

	const long result = EndSceneOriginal(currentDevice);

	if (_ReturnAddress() == returnAddress)
	{
		return result;
	}

	if (!isSetup)
	{
		SetupOverlay(currentDevice);
	}

	if (!isSetup)
	{
		return result;
	}

	if (!EnsureDeviceBinding(currentDevice))
	{
		return result;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.WantCaptureKeyboard = gui::open;
	io.MouseDrawCursor = gui::open;

	ImGui::NewFrame();

	if (gui::open)
	{
		gui::Render();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return result;
}

LRESULT CALLBACK WindowProcess(
	const HWND hwnd,
	const UINT message,
	const WPARAM wideParam,
	const LPARAM longParam)
{
	if (shuttingDown)
	{
		if (originalWindowProcess != nullptr)
		{
			return CallWindowProc(
				originalWindowProcess,
				hwnd,
				message,
				wideParam,
				longParam);
		}

		return DefWindowProc(hwnd, message, wideParam, longParam);
	}

	if (message == WM_KEYDOWN && wideParam == VK_INSERT)
	{
		gui::open = !gui::open;
	}

	if (gui::open && ImGui::GetCurrentContext() != nullptr)
	{
		const LRESULT imguiResult = ImGui_ImplWin32_WndProcHandler(
			hwnd,
			message,
			wideParam,
			longParam);

		if (imguiResult != 0)
		{
			return imguiResult;
		}

		if (ShouldBlockGameKeys(message))
		{
			return 0;
		}
	}

	return CallWindowProc(
		originalWindowProcess,
		hwnd,
		message,
		wideParam,
		longParam);
}
