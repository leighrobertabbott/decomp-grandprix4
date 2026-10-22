#include <gp4/gp4.h>

// One helper per arm so the compiler keeps both call addresses as written.
static __declspec(noinline) void DrawWithGdi(gp4::Regs& c, const uint32_t (&args)[3]) {
    gp4::call_regs_stdcall(0x005083c0, c, args);
}
static __declspec(noinline) void DrawIntoBuffer(gp4::Regs& c, const uint32_t (&args)[3]) {
    gp4::call_regs_stdcall(0x0054654a, c, args);
}

// Literal-text handler: draws the string at esi at the cursor pair (x = [0x831560], y = [0x831564]),
// through GDI TextOutA (0x005083c0) when the byte [0x007ab53c] is zero, else into the text buffer
// (0x0054654a), then advances esi over the drawn run of bytes >= 0x14 (0x00546154). Registers and
// flags thread from the drawer into the cursor skip; everything comes back from the latter.
// FUNCTION: GP4 0x0054611f
void __cdecl FUN_0054611f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[3] = {GP4_GLOBAL(uint32_t, 0x00831560), GP4_GLOBAL(uint32_t, 0x00831564), c.esi};
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0u) {
        DrawWithGdi(c, args);
    } else {
        DrawIntoBuffer(c, args);
    }
    gp4::call_regs(0x00546154, c);
    *r = c;
}
GP4_IMPL(0x0054611f, FUN_0054611f, "regs(esi:ptr[0x200]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_00545f09+surf_0054654a; emu=skip:win32 TextOutA")
