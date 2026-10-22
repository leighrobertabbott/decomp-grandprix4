#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b94d0 { uint8_t Unknown00[4]; void* Self04; uint8_t Unknown08[0x8]; };
#pragma pack(pop)

// Points the field at +4 at the sub-object at +8 and returns this
// (`mov eax, ecx; lea ecx, [eax+8]; mov [eax+4], ecx; ret`).
// FUNCTION: GP4 0x005b94d0
Unknown_005b94d0* __fastcall FUN_005b94d0(Unknown_005b94d0* self, void* /*edx*/) {
    self->Self04 = self->Unknown08;
    return self;
}
GP4_IMPL(0x005b94d0, FUN_005b94d0, "thiscall(ptr[0x10]:bytes) -> i32")
