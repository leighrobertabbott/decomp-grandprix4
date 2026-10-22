#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00471ac0 { uint8_t Failed00; uint8_t Unknown01[7]; void* File08; };
#pragma pack(pop)

// Reads one item from the stream held at +8 with the format string at 0x0064299c;
// Failed00 is 0 when the read matched something, 1 on 0 / EOF.
// FUN_00471a80 writes the same stream with the same format string.
// FUNCTION: GP4 0x00471ac0
int __fastcall FUN_00471ac0(Unknown_00471ac0* self, void* /*edx*/, void* out) {
    int n = gp4::cdecl_<0x005dd16e, int>(self->File08, (const char*)0x0064299c, out);
    if (n != 0 && n != -1) {
        self->Failed00 = 0;
        return (int)self;
    }
    self->Failed00 = 1;
    return (int)self;
}
GP4_IMPL(0x00471ac0, FUN_00471ac0, "thiscall(ptr[0x100]:bytes, ptr[0x10]:bytes) -> i32")
