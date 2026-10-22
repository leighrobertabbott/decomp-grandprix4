#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004e8a90 {
    uint8_t Unknown00[0xc];
    void* Block0c;
    void* Block10;
};
#pragma pack(pop)

// Releases the two process-heap blocks at +0xc and +0x10 (when set) and clears the pointers.
// FUNCTION: GP4 0x004e8a90
void __fastcall FUN_004e8a90(Unknown_004e8a90* self, void* /*edx*/) {
    if (self->Block0c != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block0c);
        self->Block0c = 0;
    }
    if (self->Block10 != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block10);
        self->Block10 = 0;
    }
}
GP4_IMPL(0x004e8a90, FUN_004e8a90, "thiscall(ptr[0x20]:u32{0}) -> void")
