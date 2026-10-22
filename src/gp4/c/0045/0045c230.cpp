#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Object_0045c230 {
    const void* Vptr00;
    uint8_t Unknown04[8];
    void* Buffer0C;
    uint8_t Unknown10[0x10];
    void* Buffer20;
};
#pragma pack(pop)

// Scalar deleting destructor: restores the vtable pointer 0x005f05cc, frees the two heap buffers
// at +0x0c and +0x20 (when set), and frees the object itself when bit 0 of the flag is set.
// FUNCTION: GP4 0x0045c230
Object_0045c230* __fastcall FUN_0045c230(Object_0045c230* self, void* /*edx*/, uint8_t flags) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f05cc);
    if (self->Buffer0C) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Buffer0C);
        self->Buffer0C = 0;
    }
    if (self->Buffer20) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self->Buffer20);
        self->Buffer20 = 0;
    }
    if (flags & 1) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self);
    }
    return self;
}
GP4_IMPL(0x0045c230, FUN_0045c230, "thiscall(ptr[0x28]:{0xc:nptr[0x20]:bytes, 0x20:nptr[0x20]:bytes}, u32{0,1,2,3}) -> i32")
