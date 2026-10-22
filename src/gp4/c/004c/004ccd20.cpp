#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004ccd20 { uint8_t Unknown00[0x94]; uint8_t Flag94; };
#pragma pack(pop)

// Returns the byte at +0x94 of the object (mov al, [ecx+0x94]).
// FUNCTION: GP4 0x004ccd20
uint8_t __fastcall FUN_004ccd20(Unknown_004ccd20* self, void* /*edx*/) {
    return self->Flag94;
}
GP4_IMPL(0x004ccd20, FUN_004ccd20, "thiscall(ptr[0xa0]:bytes) -> u8")
