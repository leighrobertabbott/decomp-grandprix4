#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets up the segment flag-pattern writer 0x00557083: dl = 0x20 when cl == 8, else 0x10 (the channel bit);
// the word at [esi] (esi advances by 2) is the repeat count [0x00874b50]; [0x00874b58] = 2 (records per
// repeat), [0x00874b59] = 1, [0x00874b4e] = 8 and [0x00874b3e] = 0. The registers and flags come back from
// 0x00557083 (eax, ebx, edx).
// FUNCTION: GP4 0x00556fe9
void __cdecl FUN_00556fe9(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t channel = c.ecx & 0xffu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(channel, 8u));
    c.edx = (c.edx & 0xffffff00u) | ((channel == 8u) ? 0x20u : 0x10u);

    GP4_GLOBAL(uint16_t, 0x00874b50) = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.esi));
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.esi, 2u));
    c.esi += 2u;
    GP4_GLOBAL(uint8_t, 0x00874b58) = 2u;
    GP4_GLOBAL(uint8_t, 0x00874b59) = 1u;
    GP4_GLOBAL(uint16_t, 0x00874b4e) = 8u;
    GP4_GLOBAL(uint8_t, 0x00874b3e) = 0u;
    gp4::call_regs(0x00557083, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00556fe9, FUN_00556fe9, "regs(eax:u32, ecx:u32{0x8,0x108,0x10,0x0}, edx:u32, esi:ptr[0x10]:u16[1..3], ebx:rec[0x8943a0,0x160,0..25], edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+bits_0055616f+flags_00874b3e+pan_0055933e+walk_00557083")
