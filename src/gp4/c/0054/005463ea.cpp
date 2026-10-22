#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) void drawGdi(gp4::Regs& c, const uint32_t (&args)[3]) {
    gp4::call_regs_stdcall(0x005083c0, c, args);
}

static __declspec(noinline) void drawSurface(gp4::Regs& c, const uint32_t (&args)[3]) {
    gp4::call_regs_stdcall(0x0054654a, c, args);
}

// Formats the number eax into the text buffer 0x0083156c with the decimal formatter 0x005464aa (edi =
// the buffer), then draws that text at the position (dword 0x00831560, dword 0x00831564): through the
// GDI text routine 0x005083c0 when the byte 0x007ab53c is zero, else through the surface text copier
// 0x0054654a. Registers and flags come back from the last callee (stdcall, arguments x, y, text).
// FUNCTION: GP4 0x005463ea
void __cdecl FUN_005463ea(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edi = 0x0083156cu;
    gp4::call_regs(0x005464aa, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007ab53c), 0u));

    const uint32_t args[3] = {GP4_GLOBAL(uint32_t, 0x00831560), GP4_GLOBAL(uint32_t, 0x00831564), 0x0083156cu};
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0u) {
        drawGdi(c, args);
    } else {
        drawSurface(c, args);
    }
    *r = c;
}
GP4_IMPL(0x005463ea, FUN_005463ea, "regs(eax:u32{0,1,5,9,10,99,100,12345,0xfffffffb,0xffffffff,0xffff0000,0x7fffffff,0x80000000,1000000}, ebx:u32{0x0030,0x0020,0x4030,0x4020,0x8030,0x8020,0xc030,0xc020,0xe030,0xe020,0x2030,0x6030,0xc000,0x8000}, ecx:u32[0..0x20]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=surf_0054654a; emu=skip:win32 TextOutA")
