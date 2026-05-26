#pragma once

#include <d3d9.h>
#include <Windows.h>

inline bool isSetup = false;
inline bool shuttingDown = false;

inline HWND window = nullptr;
inline WNDPROC originalWindowProcess = nullptr;

inline IDirect3DDevice9* device = nullptr;
inline UINT backBufferWidth = 0;
inline UINT backBufferHeight = 0;

void Setup();
void Destroy() noexcept;

void OnBeforeReset() noexcept;
bool TryReattachWindow(HWND newWindow) noexcept;
bool TryRefreshDevice(IDirect3DDevice9* newDevice) noexcept;
bool EnsureDeviceBinding(IDirect3DDevice9* currentDevice) noexcept;
void SetupOverlay(IDirect3DDevice9* currentDevice) noexcept;
void AbortOverlaySetup() noexcept;
void TeardownOverlay() noexcept;

constexpr void* VirtualFunction(void* thisptr, size_t index) noexcept
{
	return (*static_cast<void***>(thisptr))[index];
}

using EndSceneFn = long(__stdcall*)(IDirect3DDevice9*) noexcept;
inline EndSceneFn EndSceneOriginal = nullptr;
long __stdcall EndScene(IDirect3DDevice9* currentDevice) noexcept;

using ResetFn = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
inline ResetFn ResetOriginal = nullptr;
HRESULT __stdcall Reset(
	IDirect3DDevice9* currentDevice,
	D3DPRESENT_PARAMETERS* params) noexcept;

LRESULT CALLBACK WindowProcess(
	HWND hwnd,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam);
