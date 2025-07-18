#pragma once
#include <D3D11.h>
#include <dxgi.h>
#include <algorithm>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include "memory.hpp"
#include "vmt.hpp"
#include "xor.hpp"
#include "gui.hpp"
#include "source2/handle.hpp"
#include "source2/vector.hpp"
#include "source2/viewmatrix.hpp"
#include "source2/interface.hpp"
#include "features/cache.hpp"
#include "features/aim.hpp"
#include "features/esp.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef void(__thiscall* FrameStageNotify)(void* _this, int curStage);
FrameStageNotify OriginalFrameStageNotify = nullptr;
static VTableHook<FrameStageNotify> FrameStageNotifyHook;

typedef bool(__thiscall* CreateMove)(CCSGOInput* input, int seq, bool active);
CreateMove OriginalCreateMove = nullptr;
static VTableHook<CreateMove> CreateMoveHook;

namespace Hooks {
	typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
	Present OriginalPresent = nullptr;
	HWND Window = NULL;
	WNDPROC OriginalWndProc;
	bool Init = false;
	bool OpenMenu = true;

	LRESULT HkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (true && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

		return CallWindowProc(OriginalWndProc, hWnd, msg, wParam, lParam);
	}

	HRESULT __stdcall HkPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
		if (!Init) {
			if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice))) {
				g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);

				DXGI_SWAP_CHAIN_DESC sd;
				SwapChain->GetDesc(&sd);
				Window = sd.OutputWindow;

				ID3D11Texture2D* pBackBuffer;
				SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
				g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
				pBackBuffer->Release();

				OriginalWndProc = (WNDPROC)SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)HkWndProc);

				ImGui::CreateContext();
				ImGuiStyle& style = ImGui::GetStyle();
				style.WindowRounding = 2.5f;
				style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10, 255);
				style.Colors[ImGuiCol_TitleBg] = ImColor(48, 85, 255, 255);
				style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(48, 85, 255, 255);
				style.Colors[ImGuiCol_TitleBgActive] = ImColor(48, 85, 255, 255);

				ImGuiIO& io = ImGui::GetIO();
				io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
				ImFontAtlas* fontAtlas = io.Fonts;
				ImFontConfig arialConfig;
				arialConfig.FontDataOwnedByAtlas = false;
				ImFont* arialFont = fontAtlas->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdana.ttf", 14.0f, &arialConfig);
				io.Fonts = fontAtlas;
				
				ImGui_ImplWin32_Init(Window);
				ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

				Init = true;
			}
			else {
				return OriginalPresent(SwapChain, SyncInterval, Flags);
			}
		}

		if (GetAsyncKeyState(VK_INSERT) & 1) OpenMenu = !OpenMenu;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ESP::Render();

		if (Settings::FOVCircle) {
			ImVec2 ScreenCenter = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
			float ScreenHeight = ImGui::GetIO().DisplaySize.y;
			float Radius = tanf((Settings::FOV * 0.5f) * (PI / 180.0f)) * (ScreenHeight / 2.0f);

			ImGui::GetForegroundDrawList()->AddCircle(ScreenCenter, Radius, Settings::FOVCircleColor, 64, 1.0f);
		}

		if (OpenMenu) GUI::Render();

		ImGui::Render();
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return OriginalPresent(SwapChain, SyncInterval, Flags);
	}

	void __fastcall HkFrameStageNotify(void* _this, int curStage) {
		OriginalFrameStageNotify(_this, curStage);

		if (curStage == 9) {
			Cache::Run();
		}
	}

	bool __fastcall HkCreateMove(CCSGOInput* input, int seq, bool active) {
		const bool Return = OriginalCreateMove(input, seq, active);

		Aim::Run(input);

		return Return;
	}

	void Initialize() {
		uintptr_t PresentHk = Memory::FindPattern(X("GameOverlayRenderer64.dll"), X("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 41 8B E8"));
		uintptr_t CreateHk = Memory::FindPattern(X("GameOverlayRenderer64.dll"), X("48 89 5C 24 ? 57 48 83 EC ? 33 C0 48 89 44 24"));

		__int64(__fastcall* CreateHook)(unsigned __int64 pFuncAddress, __int64 pDetourFuncAddress, unsigned __int64* pOriginalFuncAddress, int a4);
		CreateHook = (decltype(CreateHook))CreateHk;
		CreateHook(PresentHk, (__int64)&HkPresent, (unsigned __int64*)&OriginalPresent, 1);

		FrameStageNotifyHook.Hook(Interface::Source2Client, 36, HkFrameStageNotify, &OriginalFrameStageNotify);
		CreateMoveHook.Hook(Interface::CSGOInput, 5, HkCreateMove, &OriginalCreateMove);
	}
}
