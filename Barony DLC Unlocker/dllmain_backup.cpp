// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "mem.h"

HMODULE DLLhModule = nullptr;

const uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"barony.exe");

HMODULE baronyModule = GetModuleHandle(L"barony.exe");

// locations

const uintptr_t DLC_overwrite_01 = (uintptr_t)(mem::AOBScan(baronyModule, "88 05 ?? ?? ?? ?? 4C 8D 25")); // replaces flag every frame @ instructions
const uintptr_t DLC_overwrite_02 = (uintptr_t)(mem::AOBScan(baronyModule, "88 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? FF 15")); // replaces flag every frame @ instructions

uintptr_t DLC_01_bool;
uintptr_t DLC_02_bool;

// original opcode

char* dlc_01_orig;
char* dlc_02_orig;

// toggle

bool patchToggle = false;

uintptr_t ResolveRelativeAddress(uintptr_t instructionAddress, int instructionSize, int offsetToDisplacement) {

    // 1. Get a pointer to where the 4 displacement bytes start
    uintptr_t displacementPtr = instructionAddress + offsetToDisplacement;

    // 2. Read the 4 bytes as a signed 32-bit integer.
    // The CPU automatically reads 'd1 48 2b 00' as 0x002B48D1 here. No manual reversal needed!
    int32_t displacement = *(int32_t*)displacementPtr;

    // 3. Calculate the RIP-Relative address
    // Target = InstructionStart + InstructionSize + Displacement
    uintptr_t targetAddress = instructionAddress + instructionSize + displacement;

    return targetAddress;
}

//char* ToHexEscapeCharPtr(uintptr_t value, int numBytes) {
//    // Each byte needs 4 characters: "\xNN" 
//    // Plus 1 byte for the null terminator '\0'
//    int bufferSize = (numBytes * 4) + 1;
//    char* buffer = new char[bufferSize];
//
//    char* currentPos = buffer;
//    for (int i = 0; i < numBytes; ++i) {
//        unsigned char byte = (value >> (i * 8)) & 0xFF;
//
//        // sprintf writes exactly 4 chars: \ x 0 0
//        sprintf(currentPos, "\\x%02x", byte);
//        currentPos += 4;
//    }
//
//    return buffer; // Caller is responsible for delete[]
//}

DWORD WINAPI HackThread(LPVOID lpReserved)
{

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "==== DLC Unlocker attached! ====\n\n";
    std::cout << "Press Numpad 0 to enable the patch.\nPress DEL to kill.\n\n";

    try
    {
        DLC_01_bool = ResolveRelativeAddress(DLC_overwrite_01, 6, 2);

        DLC_02_bool = ResolveRelativeAddress(DLC_overwrite_02, 6, 2);

        //dlc_01_orig = ToHexEscapeCharPtr(DLC_01_bool, 6);

        //dlc_02_orig = ToHexEscapeCharPtr(DLC_02_bool, 6);
    }
    catch (int errorCode)
    {
        std::cout << "ERROR fetching dlc bool or string: " << errorCode << std::endl;
    }

    while (true)
    {
        if (GetAsyncKeyState(VK_NUMPAD0) & 1)
        {
            patchToggle = !patchToggle;

            if (patchToggle)
            {
                //mem::Patch(((BYTE*)DLC_Validation), (BYTE*)("\xB8\x01\x00\x00\x00"), 5);
                //mem::Patch(((BYTE*)DLC_Validation+5), (BYTE*)("\xC3"), 1);
                mem::Nop((BYTE*)DLC_overwrite_01, 6);
                mem::Nop((BYTE*)DLC_overwrite_02, 6);
                
                bool* test = (bool*)DLC_01_bool;
                *test = 1;

                test = (bool*)DLC_02_bool;
                *test = 1;
            }
            else
            {
                /*mem::Patch(((BYTE*)DLC_overwrite_01), (BYTE*)(dlc_01_orig), 6);
                mem::Patch(((BYTE*)DLC_Validation + 5), (BYTE*)("\x48\x89\x74\x24\x18"), 5);

                mem::Patch(((BYTE*)DLC_overwrite_01), (BYTE*)("\x75"), 1);
                mem::Patch(((BYTE*)DLC_overwrite_02), (BYTE*)("\x75"), 1);*/
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