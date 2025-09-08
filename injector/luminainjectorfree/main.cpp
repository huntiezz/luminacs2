#define _HAS_STD_BYTE  0
#include "x.hpp"
#include "bytes.hpp"
#include "inj.h"

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

void lol() {
    char title[128];
    sprintf_s(title, sizeof(title), X("luminacheats.com | Built: %s %s | Version: Free CS2"), __DATE__, __TIME__);
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
    int Option = 0;

    lol();
    scanf_s(X("%d"), &Option);

    if (Option == 1) {
        system(X("cls"));

        char Buffer[128];
        printf(X("\nEnter key:\n"));
        scanf_s("%127s", Buffer, (unsigned)_countof(Buffer));

        std::string K = Buffer;
        if (K == X("Luminaaaaa")) {
            system(X("cls"));

            DWORD PID = GetProcessIdByName((wchar_t*)X(L"cs2.exe"));
            if (PID == 0) {
                printf(X("[ lumina ] CS2 is not running. Please make sure it is open!\n"));
                system(X("pause"));

                return NULL;
            }

            HANDLE Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

            Sleep(2500);

            printf(X("[ lumina ] Injecting into CS2...\n"));

            Sleep(2500);

            if (!Map(Process, (BYTE*)DllData, sizeof(DllData))) {
                printf(X("[ lumina ] Injection failed...\n"));
                CloseHandle(Process);
                system(X("pause"));

                return NULL;
            }

            printf(X("[ lumina ] Injection successful!\n"));
            CloseHandle(Process);
            system(X("pause"));
        }
        else {
            printf(X("[ lumina ] Invalid key!\n"));
            system(X("pause"));
        }
    }
    else if (Option == 2) {
        printf(X("[ lumina ] Exiting in 1s\n"));
        Sleep(1000);
        exit(0);
    }
    else {
        printf(X("[ lumina ] Invalid selection...\n"));
        system(X("pause"));
    }

    return NULL;
}
