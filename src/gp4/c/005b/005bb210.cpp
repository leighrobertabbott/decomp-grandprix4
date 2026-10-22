#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bb210 { uint32_t Value00; uint32_t Value04; uint32_t Unknown08; uint32_t Value0c; };
#pragma pack(pop)

// Constructor-like clear: zeroes the dwords at +0, +4 and +0xc, returns this.
// FUNCTION: GP4 0x005bb210
Unknown_005bb210* __fastcall FUN_005bb210(Unknown_005bb210* self, void* /*edx*/) {
    self->Value00 = 0;
    self->Value04 = 0;
    self->Value0c = 0;
    return self;
}
GP4_IMPL(0x005bb210, FUN_005bb210, "thiscall(ptr[0x18]:bytes) -> i32")
