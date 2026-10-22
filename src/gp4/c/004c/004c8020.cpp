#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004c8020 {
    uint32_t Vtable00;
    uint8_t Unknown04[0x144];
    void* Buffer148;
    uint32_t Unknown14c;
    uint32_t Unknown150;
};
#pragma pack(pop)

// Resets the object's vtable pointer to 0x005f0cac, frees the heap block at +0x148 with
// HeapFree(GetProcessHeap(), 0, ...) and zeroes the three dwords at +0x148, +0x14c and +0x150.
// FUNCTION: GP4 0x004c8020
void __fastcall FUN_004c8020(Unknown_004c8020* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0cac;
    GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Buffer148);
    self->Buffer148 = nullptr;
    self->Unknown14c = 0;
    self->Unknown150 = 0;
}
GP4_IMPL(0x004c8020, FUN_004c8020, "thiscall(ptr[0x158]:bytes) -> void")
