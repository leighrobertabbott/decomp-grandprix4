#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeap_t)();
typedef int (__stdcall *HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004db960 {
    uint8_t Unknown00[0x44];
    void* Block44;
    void* Block48;
    uint8_t Unknown4c[4];
    void* Block50;
    uint8_t Unknown54[4];
    void* Block58;
};
#pragma pack(pop)

// Releases the four process-heap blocks held at +0x50, +0x58, +0x44 and +0x48 of the object (in that
// order) and clears each pointer; null pointers are skipped.
// FUNCTION: GP4 0x004db960
void __fastcall FUN_004db960(Unknown_004db960* self) {
    if (self->Block50 != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block50);
        self->Block50 = 0;
    }
    if (self->Block58 != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block58);
        self->Block58 = 0;
    }
    if (self->Block44 != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block44);
        self->Block44 = 0;
    }
    if (self->Block48 != 0) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Block48);
        self->Block48 = 0;
    }
}
GP4_IMPL(0x004db960, FUN_004db960, "thiscall(ptr[0x70]:{0x44:nptr[0x20]:bytes, 0x48:nptr[0x20]:bytes, 0x50:nptr[0x20]:bytes, 0x58:nptr[0x20]:bytes}) -> void")
