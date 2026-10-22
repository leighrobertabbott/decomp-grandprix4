#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00459990 {
    uint32_t Vptr00;
    uint32_t Field04;
    uint32_t Field08;
    int32_t Value0c;
};
#pragma pack(pop)

// Constructor: stores the argument at +0xc, the vtable pointer 0x005f0584 at +0 and clears the
// dwords at +4 and +8; returns this.
// FUNCTION: GP4 0x00459990
Unknown_00459990* __fastcall FUN_00459990(Unknown_00459990* self, void* /*edx*/, int32_t value) {
    self->Value0c = value;
    self->Vptr00 = 0x005f0584;
    self->Field04 = 0;
    self->Field08 = 0;
    return self;
}
GP4_IMPL(0x00459990, FUN_00459990, "thiscall(ptr[0x10]:bytes, i32) -> u32")
