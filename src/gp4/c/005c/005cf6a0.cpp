#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cf6a0 {
    uint8_t Unknown00[0x1c];
    int32_t Value1c;
    int32_t Value20;
    int32_t Value24;
    uint8_t Unknown28[0xd8];
};
#pragma pack(pop)

// Clears the dwords at +0x20, +0x24 and +0x1c of the object and returns the object
// (`mov eax,ecx; xor ecx,ecx; mov [eax+0x20],ecx; mov [eax+0x24],ecx; mov [eax+0x1c],ecx; ret`).
// FUNCTION: GP4 0x005cf6a0
Unknown_005cf6a0* __fastcall FUN_005cf6a0(Unknown_005cf6a0* self, void* /*edx*/) {
    self->Value20 = 0;
    self->Value24 = 0;
    self->Value1c = 0;
    return self;
}
GP4_IMPL(0x005cf6a0, FUN_005cf6a0, "thiscall(ptr[0x100]:bytes) -> i32")
