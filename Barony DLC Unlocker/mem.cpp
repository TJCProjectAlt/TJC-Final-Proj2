#include "pch.h"
#include "mem.h"

namespace mem
{
	void Patch(BYTE* dst, BYTE* src, unsigned int size)
	{
		DWORD oldprotect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
		memcpy(dst, src, size);
		VirtualProtect(dst, size, oldprotect, &oldprotect);
	}

	void Nop(BYTE* dst, unsigned int size)
	{
		DWORD oldprotect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
		memset(dst, 0x90, size);
		VirtualProtect(dst, size, oldprotect, &oldprotect);
	}

	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); i++)
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
		return addr;
	}

	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len)
	{
		if (len < 5)
			return false;

		DWORD curProtection;
		VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

		uintptr_t relativeAddress = dst - src - 5;

		*src = 0xE9;

		*(uintptr_t*)(src + 1) = relativeAddress;

		VirtualProtect(src, len, curProtection, &curProtection);
	}

	BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len)
	{
		if (len < 5)
			return 0;

		// Create Gateway
		BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		// Write stole bytes to gateway
		memcpy_s(gateway, len, src, len);

		// Get gateway to destination address
		uintptr_t gatewayRelativeAddr = src - gateway - 5;

		// add the jmp opcode to the end of the gateway
		*(gateway + len) = 0xE9;

		// Write the addr of the gateway to the jmp
		*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

		// Perform the detour

		Detour32(src, dst, len);

		return gateway; 
	}

    /**
     * @brief AOB Scan
     *
     * @param hModule the handle to the module to be scaned
     * @param pattern AOB in the form of "XX XX XX XX", use "??" as wildcard
     * @return The address of the first match, null if not found
     */
    uintptr_t AOBScan(HMODULE hModule, std::string pattern)
    {
        // break the AOB into a vector of byte strings
        std::vector<std::string> patts;
        std::stringstream ss(pattern);
        std::string s;
        while (std::getline(ss, s, ' '))
        {
            patts.push_back(s);
        }
        int totlen = patts.size();

        // split the pattern into several small segments that does not contain any wildcards
        bool gapMode = false;
        std::vector<std::vector<BYTE>> bytes;  // ← STORE VECTORS, NOT POINTERS
        std::vector<int> lens;
        std::vector<int> gaps;
        std::vector<BYTE> byte;  // ← NO 'new', just stack allocation
        int gap = 0;

        for (size_t i = 0; i < patts.size(); i++)
        {
            if (patts[i] == "??")
            {
                if (!gapMode)
                {
                    if (!byte.empty())  // Only push if there's data
                    {
                        lens.push_back(byte.size());
                        bytes.push_back(byte);  // ← COPY THE VECTOR
                        byte.clear();  // Clear for reuse
                    }
                    gapMode = true;
                }
                gap++;
            }
            else
            {
                if (gapMode)
                {
                    gaps.push_back(gap);
                    gap = 0;
                    gapMode = false;
                }
                byte.push_back((BYTE)stoi(patts[i], nullptr, 16));
            }
        }

        // Push final segment
        if (!byte.empty())
        {
            lens.push_back(byte.size());
            bytes.push_back(byte);
            gaps.push_back(0);
        }

        // Ensure gaps vector matches bytes vector size
        while (gaps.size() < bytes.size())
        {
            gaps.push_back(0);
        }

        // Safety check
        if (bytes.empty() || lens.empty())
        {
            return 0;
        }

        char debugMsg[256];
        sprintf_s(debugMsg, "Pattern segments: %zu, First segment size: %zu\n",
            bytes.size(), bytes.empty() ? 0 : bytes[0].size());
        OutputDebugStringA(debugMsg);

        if (bytes.empty())
        {
            OutputDebugStringA("ERROR: No byte segments found!\n");
            return 0;
        }

        if (bytes[0].empty())
        {
            OutputDebugStringA("ERROR: First segment is empty!\n");
            return 0;
        }

        // Get module size
        MODULEINFO modInfo;
        GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO));
        DWORD moduleSize = modInfo.SizeOfImage;

        BYTE* begin = (BYTE*)hModule;
        BYTE* end = begin + moduleSize;

        // AOB Scan
        while (begin = std::search(begin, end, bytes[0].data(), bytes[0].data() + lens[0]))
        {
            if (begin + totlen >= end)
                break;

            auto i = begin + lens[0] + gaps[0];
            bool flag = false;

            for (size_t j = 1; j < bytes.size(); j++)
            {
                if (std::equal(bytes[j].data(), bytes[j].data() + lens[j], i))
                {
                    i += lens[j] + gaps[j];
                }
                else
                {
                    flag = true;
                    break;
                }
            }

            if (!flag)
                return (uintptr_t)begin;
            begin++;
        }

        // return null if no match found
        return 0;
    }
}