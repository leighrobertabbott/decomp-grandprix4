#include <gp4/gp4.h>

struct Unknown_005b9ca0;

#pragma pack(push, 1)
struct Unknown_005b9ca0 { uint8_t Unknown00[0x80]; Unknown_005b9ca0* Self80; uint32_t Value84; };
#pragma pack(pop)

// Makes the field at +0x80 point back at the object itself, clears the dword at +0x84 and returns
// the object (an empty circular-list head initialiser).
// FUNCTION: GP4 0x005b9ca0
Unknown_005b9ca0* __fastcall FUN_005b9ca0(Unknown_005b9ca0* self, void* /*edx*/) {
    self->Self80 = self;
    self->Value84 = 0;
    return self;
}
GP4_IMPL(0x005b9ca0, FUN_005b9ca0, "thiscall(ptr[0x90]:bytes) -> i32")
