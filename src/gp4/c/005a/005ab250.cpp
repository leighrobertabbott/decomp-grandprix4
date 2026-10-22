#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab250 { uint8_t Unknown00[0x3c]; int32_t Value3c; int32_t Value40; };
#pragma pack(pop)

// Stores its first stack argument at +0x40 and its second at +0x3c of the object; returns 0.
// FUNCTION: GP4 0x005ab250
int32_t __fastcall FUN_005ab250(Unknown_005ab250* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Value40 = first;
    self->Value3c = second;
    return 0;
}
GP4_IMPL(0x005ab250, FUN_005ab250, "thiscall(ptr[0x48]:bytes, i32, i32) -> i32")
