#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Block_4b3890 {
    uint32_t Unknown00;
    void* Buffer04;
};
#pragma pack(pop)

// Frees a heap block that owns a second heap buffer (+4): releases the buffer if set, then the
// block itself, and clears the pointer cell passed in.
// FUNCTION: GP4 0x004b3890
void __cdecl FUN_004b3890(Block_4b3890** cell) {
    if (*cell != nullptr) {
        void* buffer = (*cell)->Buffer04;
        if (buffer != nullptr) {
            GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, buffer);
        }
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, *cell);
        *cell = nullptr;
    }
}
GP4_IMPL(0x004b3890, FUN_004b3890, "cdecl(ptr[4]:layout=cell_004b3890) -> void")
