#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script line for a round counter: with [0x7ad894] (the round index, from 0) at or past
// [0x7c2e84] (the number of rounds) it expands text id 0xb9 at the output cursor edi; otherwise it expands id
// 0xba, prints the round number (index + 1 unless the word [0x7c2e70] is 2) with the decimal formatter
// 0x0054642c (bh = 0, no padding) and expands id 0xbb when that number is 1, else 0xbc. A zero byte
// terminates the text at the cursor. Registers and flags are those the last callee leaves; the pushes
// around the first two calls only save and restore eax.
// FUNCTION: GP4 0x00545e54
void __cdecl FUN_00545e54(gp4::Regs* r) {
    const uint32_t round = GP4_GLOBAL(uint32_t, 0x007ad894);
    const uint32_t rounds = GP4_GLOBAL(uint32_t, 0x007c2e84);
    gp4::Regs c = *r;
    c.eax = round;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(round, rounds));
    if (round >= rounds) {
        c.esi = 0xb9u;
        gp4::call_regs(0x0054d798, c);
    } else {
        c.esi = 0xbau;
        gp4::call_regs(0x0054d798, c);
        c.eax = round;
        uint32_t number = round;
        const uint16_t mode = GP4_GLOBAL(uint16_t, 0x007c2e70);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub16(mode, 2u));
        if (mode != 2u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(number, c.eflags));
            number = number + 1u;
        }
        c.eax = number;
        c.ebx = c.ebx & 0xffff00ffu;
        gp4::call_regs(0x0054642c, c);
        c.eax = number;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(number, 1u));
        c.esi = (number == 1u) ? 0xbbu : 0xbcu;
        gp4::call_regs(0x0054d798, c);
    }
    GP4_FIELD(uint8_t, c.edi, 0) = 0u;
    *r = c;
}
GP4_IMPL(0x00545e54, FUN_00545e54, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+cnt_00548836+script_all_00835846")
