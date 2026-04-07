// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "mem.h"

HMODULE DLLhModule = nullptr;

const uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"barony.exe");

HMODULE baronyModule = GetModuleHandle(L"barony.exe");

const uintptr_t DLC_Validation = (uintptr_t)(mem::AOBScan(baronyModule, "48 89 5C 24 ?? 48 89 74 24 ?? 55 57 41 56 48 8B EC 48 83 EC ?? 45 32 D2"));

const uintptr_t DLC_jmp1 = (uintptr_t)(mem::AOBScan(baronyModule, "75 ?? 8D 43 ?? 83 F8 ?? 76"));

const uintptr_t DLC_jmp2 = (uintptr_t)(mem::AOBScan(baronyModule, "75 ?? 8D 43 ?? 83 F8 ?? 77"));

bool patchToggle = false;

DWORD WINAPI HackThread(LPVOID lpReserved)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "==== DLC Unlocker attached! ====\n\n";
    std::cout << "Press Numpad 0 to toggle the patch.\nPress DEL to kill.\n\n";



    while (true)
    {
        if (GetAsyncKeyState(VK_NUMPAD0) & 1)
        {
            patchToggle = !patchToggle;

            if (patchToggle && DLC_Validation > 0)
            {
                mem::Patch(((BYTE*)DLC_Validation), (BYTE*)("\xB8\x01\x00\x00\x00"), 5);
                mem::Patch(((BYTE*)DLC_Validation+5), (BYTE*)("\xC3"), 1);
                mem::Nop((BYTE*)DLC_Validation + 6, 4);


                mem::Patch(((BYTE*)DLC_jmp1), (BYTE*)("\xEB"), 1);

                mem::Patch(((BYTE*)DLC_jmp2), (BYTE*)("\xEB"), 1);
            }
            else
            {
                mem::Patch(((BYTE*)DLC_Validation), (BYTE*)("\x48\x89\x5C\x24\x08"), 5);
                mem::Patch(((BYTE*)DLC_Validation + 5), (BYTE*)("\x48\x89\x74\x24\x18"), 5);

                mem::Patch(((BYTE*)DLC_jmp1), (BYTE*)("\x75"), 1);
                mem::Patch(((BYTE*)DLC_jmp2), (BYTE*)("\x75"), 1);
            }
        }
        if (GetAsyncKeyState(VK_DELETE) & 1)
            break;
        Sleep(50);
    }

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(DLLhModule, 0);
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DLLhModule = hModule;
        DisableThreadLibraryCalls(hModule);
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}