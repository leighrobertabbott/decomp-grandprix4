#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005bb9d0 { uint8_t Unknown00[4]; float Value04; int32_t Value08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Clears the dword at +8, then stores the result of 0x005c0260 (clock() scaled by a float, left
// on the x87 stack at full precision) as a float at +4 (fstp dword).
// FUNCTION: GP4 0x005bb9d0
void __fastcall FUN_005bb9d0(Unknown_005bb9d0* self, void* /*edx*/) {
    self->Value08 = 0;
    gp4::Regs c = {};
    gp4::call_regs(0x005c0260, c);
    self->Value04 = gp4::x87::narrow_f32(gp4::x87::out_st(c, 0));
}
GP4_IMPL(0x005bb9d0, FUN_005bb9d0, "thiscall(ptr[0x100]:bytes) -> void; emu=skip:win32 GetSystemTimeAsFileTime")
