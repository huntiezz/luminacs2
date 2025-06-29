#include <Windows.h>
#include <cstdio>
#include "hooks.hpp"

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
		case DLL_PROCESS_ATTACH: {
			DisableThreadLibraryCalls(hModule);

			Hooks::Initialize();
			Stuff::Initialize();

			break;
		}

		case DLL_PROCESS_DETACH: {
			break;
		}
	}

	return TRUE;
}