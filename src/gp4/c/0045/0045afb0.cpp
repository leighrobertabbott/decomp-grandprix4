#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045afb0 {
    uint32_t Vtable00;
    int32_t Field04;
};
#pragma pack(pop)

// Constructor-style initialiser: installs the vtable at 0x005f05bc, clears +4, returns this.
// FUNCTION: GP4 0x0045afb0
Unknown_0045afb0* __fastcall FUN_0045afb0(Unknown_0045afb0* self, void* /*edx*/) {
    self->Vtable00 = 0x005f05bc;
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x0045afb0, FUN_0045afb0, "thiscall(ptr[0x10]:bytes) -> u32")
