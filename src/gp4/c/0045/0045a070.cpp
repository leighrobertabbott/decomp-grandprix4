#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045a070 {
    const void* Vtable00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint8_t Unknown0c[4];
    uint8_t Flag10;
    uint8_t Flag11;
    uint8_t Flag12;
};
#pragma pack(pop)

// Constructor: stores the vtable pointer 0x005f0588, clears the dwords at +4 and +8 and the
// bytes at +0x10, +0x11 and +0x12, and returns the object (eax = ecx).
// FUNCTION: GP4 0x0045a070
Unknown_0045a070* __fastcall FUN_0045a070(Unknown_0045a070* self, void* /*edx*/) {
    self->Vtable00 = GP4_ARRAY(const uint8_t, 0x005f0588);
    self->Unknown08 = 0;
    self->Unknown04 = 0;
    self->Flag10 = 0;
    self->Flag12 = 0;
    self->Flag11 = 0;
    return self;
}
GP4_IMPL(0x0045a070, FUN_0045a070, "thiscall(ptr[0x20]:bytes) -> i32")
