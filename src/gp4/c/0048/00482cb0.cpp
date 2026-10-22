#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);
typedef int(__stdcall* DeleteObject_t)(void* obj);

#pragma pack(push, 1)
struct Unknown_00482cb0 {
    uint8_t Unknown00[0x40490];
    void* GdiObject40490;
    uint32_t Unknown40494;
    uint8_t Unknown40498[0x20];
    void* HeapBlock404b8;
};
#pragma pack(pop)

// Frees the heap block at +0x404b8 (HeapFree on the process heap), clears +0x40494 and, if a GDI
// object handle is held at +0x40490, deletes it with DeleteObject and clears the handle.
// FUNCTION: GP4 0x00482cb0
void __fastcall FUN_00482cb0(Unknown_00482cb0* self, void* /*edx*/) {
    GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->HeapBlock404b8);
    self->Unknown40494 = 0;
    if (self->GdiObject40490 != nullptr) {
        GP4_IAT(DeleteObject_t, 0x005f00b0)(self->GdiObject40490);
        self->GdiObject40490 = nullptr;
    }
}
GP4_IMPL(0x00482cb0, FUN_00482cb0, "thiscall(ptr[0x404c0]:bytes) -> void; emu=skip:win32 DeleteObject")
