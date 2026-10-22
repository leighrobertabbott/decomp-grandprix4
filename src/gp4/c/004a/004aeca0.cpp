#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004aeca0 {
    void* Vtable00;
    uint8_t Unknown04[0xc];
    int32_t Flag10;
    uint8_t Unknown14[4];
    void* Buffer18;
    void* Buffer1c;
    void* Buffer20;
    void* Buffer24;
};
#pragma pack(pop)

// Scalar deleting destructor: stores the class vtable 0x005f0b44 at +0, frees the four heap blocks
// at +0x1c, +0x18, +0x20, +0x24 when the flag at +0x10 is set, then frees the object itself when bit 0 of
// the flags argument is set. Returns this.
// FUNCTION: GP4 0x004aeca0
Unknown_004aeca0* __fastcall FUN_004aeca0(Unknown_004aeca0* self, void* /*edx*/, uint8_t flags) {
    HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
    GetProcessHeap_t getProcessHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
    int32_t flag = self->Flag10;
    self->Vtable00 = (void*)0x005f0b44;
    if (flag != 0) {
        heapFree(getProcessHeap(), 0, self->Buffer1c);
        heapFree(getProcessHeap(), 0, self->Buffer18);
        heapFree(getProcessHeap(), 0, self->Buffer20);
        heapFree(getProcessHeap(), 0, self->Buffer24);
    }
    if (flags & 1) {
        heapFree(getProcessHeap(), 0, self);
    }
    return self;
}
GP4_IMPL(0x004aeca0, FUN_004aeca0, "thiscall(ptr[0x30]:{0x10:u32{0,1}, 0x18:nptr[0x20]:bytes, 0x1c:nptr[0x20]:bytes, 0x20:nptr[0x20]:bytes, 0x24:nptr[0x20]:bytes}, i32) -> i32")
