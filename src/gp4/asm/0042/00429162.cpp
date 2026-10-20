#include <gp4/gp4.h>

// One helper per callee keeps every call site distinct (the compiler would otherwise select the
// callee address with a conditional move and hide it from the shape audit).
static __declspec(noinline) void blendDirect(gp4::Regs& s) { gp4::call_regs(0x00403b65, s); }
static __declspec(noinline) void blendFloored(gp4::Regs& s) { gp4::call_regs(0x00403c4c, s); }
static __declspec(noinline) void ratioClamped(gp4::Regs& s) { gp4::call_regs(0x00403ab9, s); }
static __declspec(noinline) void ratioBanded(gp4::Regs& s) { gp4::call_regs(0x00403bb6, s); }

// Segment-ring blend driver. edi is a ring position, eax a value, edx a selector (sign) and
// the input eax is kept in ebp for the blend routines. Runs the range test 0x00403850 (sets the
// byte 0x00601ba4), blends the value 0 with the segment record chosen by it (0x00403b65 or
// 0x00403c4c depending on the byte 0x00601ba6), offsets that by 0x00429131 for the next record,
// feeds the offset (ecx) through 0x00403938 (mode 7) and finishes with the ratio routine
// 0x00403ab9 or 0x00403bb6. ebx, edi and ebp are restored; ecx = 7 on return, eax / edx / flags
// come from the last callee.
// FUNCTION: GP4 0x00429162
void __cdecl FUN_00429162(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.ebp = s.eax;
    gp4::call_regs(0x00403850, s);

    if (GP4_GLOBAL(uint8_t, 0x00601ba4) != 0) {
        s.edi = GP4_GLOBAL(uint32_t, 0x00891500);
    } else {
        s.edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
    }
    s.eax = 0;
    if (GP4_GLOBAL(uint8_t, 0x00601ba6) != 0) {
        blendFloored(s);
    } else {
        blendDirect(s);
    }

    if (GP4_GLOBAL(uint8_t, 0x00601ba4) != 0) {
        s.edi = GP4_GLOBAL(uint32_t, 0x00866f00);
    } else {
        s.edi = GP4_GLOBAL(uint32_t, 0x00866f04) - 0x160u;
    }
    gp4::call_regs(0x00429131, s);

    s.ecx = s.eax;
    s.eax = 0;
    s.edx = 7;
    gp4::call_regs(0x00403938, s);

    if (GP4_GLOBAL(uint8_t, 0x00601ba6) != 0) {
        ratioBanded(s);
    } else {
        ratioClamped(s);
    }

    r->eax = s.eax;
    r->ecx = 7;
    r->edx = s.edx;
    r->eflags = s.eflags;
}
GP4_IMPL(0x00429162, FUN_00429162, "regs(eax:i32[-0x80000..0x80000], edx:i32, edi:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+widths_00852c44+ring_008943a0+records_008943a0+div_00403bb6+words_008943a0")
