#pragma once
#include <cstdint>
#include <memory>

template<typename Type>
class VTableHook {
public:
    bool Hook(void* Object, int Index, void* Function, Type* Original) {
        if (!Object || Index < 0 || !Function || !Original) return false;

        this->VirtualTable = *reinterpret_cast<uintptr_t**>(Object);

        if (this->LastHookedFunctionAddress && this->LastHookedFunctionIndex >= 0) {
            if (this->LastHookedFunctionAddress == this->VirtualTable[this->LastHookedFunctionIndex]) return false;
        }

        if (reinterpret_cast<void*>(this->VirtualTable[Index]) != Function) {
            this->VTableSize = 0;
            while (this->VirtualTable[this->VTableSize]) {
                ++this->VTableSize;
            }

            if (Index < this->VTableSize) {
                this->AllocatedVTable = static_cast<uintptr_t*>(std::malloc(this->VTableSize * sizeof(uintptr_t)));
                if (!this->AllocatedVTable) return false;

                for (int i = 0; i < this->VTableSize; ++i) {
                    this->AllocatedVTable[i] = this->VirtualTable[i];
                }

                *Original = reinterpret_cast<Type>(this->VirtualTable[Index]);
                this->AllocatedVTable[Index] = reinterpret_cast<uintptr_t>(Function);

                *reinterpret_cast<uintptr_t**>(Object) = this->AllocatedVTable;

                this->LastHookedFunctionAddress = reinterpret_cast<uintptr_t>(Function);
                this->LastHookedFunctionIndex = Index;

                return true;
            }
        }

        return false;
    }

private:
    uintptr_t* VirtualTable = nullptr;
    uintptr_t* AllocatedVTable = nullptr;
    uintptr_t LastHookedFunctionAddress = 0;
    int LastHookedFunctionIndex = -1;
    int VTableSize = 0;
};