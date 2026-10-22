#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00471a80 { uint8_t Unknown00[8]; void* File08; };
#pragma pack(pop)

// Rewinds to the end of the stream held at +8, then writes one float value (as a double)
// with the format string at 0x0064299c. Returns the object.
// FUNCTION: GP4 0x00471a80
int __fastcall FUN_00471a80(Unknown_00471a80* self, void* /*edx*/, float value) {
    gp4::cdecl_<0x005dcb47, int>(self->File08, 0, 2);
    gp4::cdecl_<0x005dca51, int>(self->File08, (const char*)0x0064299c, (double)value);
    return (int)self;
}
GP4_IMPL(0x00471a80, FUN_00471a80, "thiscall(ptr[0x100]:bytes, f32) -> i32")
