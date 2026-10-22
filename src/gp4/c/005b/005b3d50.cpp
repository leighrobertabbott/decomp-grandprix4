#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b3d50 { uint8_t Unknown00[0x194]; uint8_t Flag194; uint8_t Flag195; uint8_t Unknown196[0x6a]; };
#pragma pack(pop)

// Sets the bytes at +0x194 and +0x195 of the object to 1 and returns 0.
// FUNCTION: GP4 0x005b3d50
int __fastcall FUN_005b3d50(Unknown_005b3d50* self, void* /*edx*/) {
    self->Flag194 = 1;
    self->Flag195 = 1;
    return 0;
}
GP4_IMPL(0x005b3d50, FUN_005b3d50, "thiscall(ptr[0x200]:bytes) -> i32")
