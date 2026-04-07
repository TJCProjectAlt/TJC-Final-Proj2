#pragma once
#include <vector>   // lowercase

namespace mem
{
    void Patch(BYTE* dst, BYTE* src, unsigned int size);
    void Nop(BYTE* dst, unsigned int size);

    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);

    bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);
    BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);

    uintptr_t AOBScan(HMODULE hModule, std::string pattern);
}
