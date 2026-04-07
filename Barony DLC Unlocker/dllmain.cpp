// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "mem.h"

HMODULE DLLhModule{ nullptr };

const uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"barony.exe");

HMODULE baronyModule = GetModuleHandle(L"barony.exe");

// locations

const uintptr_t DLC_Achievement_Check{ (uintptr_t)(mem::AOBScan(baronyModule, "48 89 5C 24 ?? 55 57 41 56 48 8B EC 48 83 EC")) }; // checks if we have the achievement to play the class (So #########)
const uintptr_t DLC_overwrite_01 { (uintptr_t)(mem::AOBScan(baronyModule, "88 05 ?? ?? ?? ?? 40 38 35 ?? ?? ?? ?? 75 ?? 48 8D 0D ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 48 8B 08 48 8B 01 BA ?? ?? ?? ?? FF 50 ?? 88 05 ?? ?? ?? ?? 40 38 35")) }; // replaces flag every frame @ instructions (outdated knowledge)
const uintptr_t DLC_overwrite_02 { (uintptr_t)(mem::AOBScan(baronyModule, "88 05 ?? ?? ?? ?? 40 38 35 ?? ?? ?? ?? 75 ?? 48 8D 0D ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 48 8B 08 48 8B 01 BA ?? ?? ?? ?? FF 50 ?? 88 05 ?? ?? ?? ?? 40 84 ED")) }; // replaces flag every frame @ instructions (outdated knowledge)
const uintptr_t DLC_overwrite_03 { (uintptr_t)(mem::AOBScan(baronyModule, "88 05 ?? ?? ?? ?? 40 84 ED")) }; // replaces flag every frame @ instructions
//const uintptr_t DLC_oddity{ (uintptr_t)(mem::AOBScan(baronyModule, "41 83 FE ?? 74 ?? B9")) }; // replaces the save check on new dlc
const uintptr_t DLC_oddity{ (uintptr_t)(mem::AOBScan(baronyModule, "40 53 48 83 EC ?? 0F B6 05")) }; // replaces the save check on new dlc


const uintptr_t Msg_Func = (uintptr_t)(mem::AOBScan(baronyModule, "48 89 74 24 ?? 48 89 7C 24 ?? 41 54 41 56 41 57 48 83 EC ?? 4C 8B F2 41 0F B6 F9")); // replaces flag every frame @ instructions

uintptr_t DLC_01_bool{ 0x0 };
uintptr_t DLC_02_bool{ 0x0 };
uintptr_t DLC_03_bool{ 0x0 };

// Debug

bool passedChecks{ false };

// Functions

typedef __int64(__fastcall* tMsgBox)(const char* prompt, const char* buttonText,__int64 buttonReact, int idk);
const tMsgBox oMsgBox = (tMsgBox)(Msg_Func);

uintptr_t ResolveRelativeAddress(const uintptr_t& instructionAddress, int instructionSize, int offsetToDisplacement) 
{

    // Get a pointer to where the 4 displacement bytes start
    uintptr_t displacementPtr = instructionAddress + offsetToDisplacement;

    // Read the 4 bytes as a signed 32-bit integer.
    // The CPU automatically reads 'd1 48 2b 00' as 0x002B48D1 here. No manual reversal needed!
    int32_t displacement = *(int32_t*)displacementPtr;

    // Calculate the RIP-Relative address
    // Target = InstructionStart + InstructionSize + Displacement
    uintptr_t targetAddress = instructionAddress + instructionSize + displacement;

    return targetAddress;
}

DWORD WINAPI HackThread(LPVOID lpReserved)
{

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "==== DLC Unlocker attached! ====\n\n";
    std::cout << "Press DEL to kill.\n\n";

    __try
    {
        DLC_01_bool = ResolveRelativeAddress(DLC_overwrite_01, 6, 2);

        DLC_02_bool = ResolveRelativeAddress(DLC_overwrite_02, 6, 2);

        DLC_03_bool = ResolveRelativeAddress(DLC_overwrite_03, 6, 2);

        passedChecks = 1;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        std::cout << "ERROR fetching dlc bool! Likely means the pattern is wrong!\nPlease attach debugger\n\n";
    }

    if (passedChecks)
    {
        //std::cout << std::hex << "0x" << DLC_oddity << "\n";
        mem::Patch(((BYTE*)DLC_Achievement_Check), (BYTE*)("\xB8\x01\x00\x00\x00"), 5);
        mem::Patch(((BYTE*)DLC_Achievement_Check + 5), (BYTE*)("\xC3"), 1);
        //mem::Patch(((BYTE*)DLC_oddity + 4), (BYTE*)("\x75"), 1);
        mem::Patch(((BYTE*)DLC_oddity), (BYTE*)("\xB8\x01\x00\x00\x00"), 5);
        mem::Patch(((BYTE*)DLC_oddity + 5), (BYTE*)("\xC3"), 1);
        mem::Nop((BYTE*)DLC_oddity + 6, 1 );
        mem::Nop((BYTE*)DLC_overwrite_01, 6);
        mem::Nop((BYTE*)DLC_overwrite_02, 6);
        mem::Nop((BYTE*)DLC_overwrite_03, 6);

        bool* test = (bool*)DLC_01_bool;
        *test = 1;

        test = (bool*)DLC_02_bool;
        *test = 1;

        test = (bool*)DLC_03_bool;
        *test = 1;

        oMsgBox("All DLC unlocked!\nEnjoy, chud!", "Close", (moduleBase + 0x8B2A30), 0);
        std::cout << "\n=== Console Output ===\n";
    }

    while (true)
    {
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