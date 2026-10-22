#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Triple_00491c50 { uint32_t Word00; uint32_t Word04; uint32_t Word08; };
#pragma pack(pop)

// Copy constructor of a 12-byte value: copies three dwords from the stack argument to the object
// and returns the object (eax = ecx).
// FUNCTION: GP4 0x00491c50
Triple_00491c50* __fastcall FUN_00491c50(Triple_00491c50* self, void* /*edx*/, const Triple_00491c50* src) {
    self->Word00 = src->Word00;
    self->Word04 = src->Word04;
    self->Word08 = src->Word08;
    return self;
}
GP4_IMPL(0x00491c50, FUN_00491c50, "thiscall(ptr[0x10]:bytes, ptr[0x10]:bytes) -> u32")
