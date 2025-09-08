#pragma once
#include <Psapi.h>
#include <string>

#define CS_CONCATENATE_DETAIL(x, y) x##y
#define CS_CONCATENATE(x, y) CS_CONCATENATE_DETAIL(x, y)
#define MEM_PAD(SIZE) \
private: \
    char CS_CONCATENATE(pad_, __COUNTER__)[SIZE]; \
public:
#define SCHEMA_ADD_OFFSET(TYPE, NAME, OFFSET)                                                                 \
	[[nodiscard]] inline std::add_lvalue_reference_t<TYPE> NAME()                                          \
	{                                                                                                         \
		static const std::uint32_t uOffset = OFFSET;                                                          \
		return *reinterpret_cast<std::add_pointer_t<TYPE>>(reinterpret_cast<std::uint8_t*>(this) + (uOffset)); \
	}

#define IN_RANGE(X, A,B) (X >= A && X <= B)
#define GetBits(X) (IN_RANGE((X & (~0x20)), 'A', 'F') ? ((X & (~0x20)) - 'A' + 0xA): (IN_RANGE(X, '0', '9') ? X - '0': 0))
#define GetByte(X) (GetBits(X[0]) << 4 | GetBits(X[1]))

namespace Memory {
    uintptr_t FindPattern(const uintptr_t& StartAddress, const uintptr_t& EndAddress, const char* PatternTarget) {
        const char* Pattern = PatternTarget;
        uintptr_t FirstMatch = 0;

        for (uintptr_t Pos = StartAddress; Pos < EndAddress; Pos++) {
            if (!*Pattern) return FirstMatch;

            const uint8_t CurrentPattern = *reinterpret_cast<const uint8_t*>(Pattern);
            const uint8_t CurrentMemory = *reinterpret_cast<const uint8_t*>(Pos);

            if (CurrentPattern == '\?' || CurrentMemory == GetByte(Pattern)) {
                if (!FirstMatch) FirstMatch = Pos;
                if (!Pattern[2]) return FirstMatch;

                Pattern += CurrentPattern != '\?' ? 3 : 2;
            }
            else {
                Pattern = PatternTarget;
                FirstMatch = 0;
            }
        }

        printf("Pattern \"%s\" not found\n", PatternTarget);

        return NULL;
    }

    uintptr_t FindPattern(const char* Module, const char* Pattern) {
        MODULEINFO ModuleInfo = { 0 };

        if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(Module), &ModuleInfo, sizeof(MODULEINFO))) return NULL;

        const uintptr_t StartAddress = uintptr_t(ModuleInfo.lpBaseOfDll);
        const uintptr_t EndAddress = StartAddress + ModuleInfo.SizeOfImage;

        return FindPattern(StartAddress, EndAddress, Pattern);
    }

    uintptr_t GetAbsoluteAddress(uintptr_t Address, const int PreOffset, const int PostOffset) {
        Address += PreOffset;
        int32_t nRva = *reinterpret_cast<int32_t*>(Address);
        Address += PostOffset + sizeof(uint32_t) + nRva;

        return Address;
    }

    uintptr_t GetRelativeAddress(uintptr_t Address, uint32_t RVAOffset, uint32_t RIPOffset) {
        uint32_t RVA = *reinterpret_cast<uint32_t*>(Address + RVAOffset);
        uintptr_t RIP = Address + RIPOffset;

        return RVA + RIP;
    }
}