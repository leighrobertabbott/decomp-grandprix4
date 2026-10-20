#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream handler for the segment flag-pattern writer 0x00557083, active only when the mode byte
// [0x0086857c] is 2 (otherwise it just skips the ten stream bytes: esi += 0xa, flags of that add).
// The ten stream bytes at [esi] are five words: +0 -> [0x00874b4e] (low byte table index, high byte
// 0x00874b4f flag bits), +2 -> repeat count [0x00874b50], +4 -> [0x00874b52], +6 -> [0x00874b56], +8 ->
// the step bytes [0x00874b58] / [0x00874b59] (ax is left holding that word). dl is the channel bit:
// 0x80 when cl == 0x0a, else 0x40; [0x00874b3e] = 0x80. The registers and flags come back from 0x00557083
// (eax, ebx, edx), esi has advanced by ten and the flags of `add esi, 0xa` reach the callee.
// FUNCTION: GP4 0x00557023
void __cdecl FUN_00557023(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint32_t stream = c.esi;

    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        r->esi = stream + 0xau;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(stream, 0xau));
        return;
    }

    const uint32_t channel = ((c.ecx & 0xffu) == 0xau) ? 0x80u : 0x40u;
    c.edx = (c.edx & 0xffffff00u) | channel;

    GP4_GLOBAL(uint16_t, 0x00874b4e) = GP4_FIELD(uint16_t, stream, 0);
    GP4_GLOBAL(uint16_t, 0x00874b50) = GP4_FIELD(uint16_t, stream, 2);
    GP4_GLOBAL(uint16_t, 0x00874b52) = GP4_FIELD(uint16_t, stream, 4);
    GP4_GLOBAL(uint16_t, 0x00874b56) = GP4_FIELD(uint16_t, stream, 6);

    const uint32_t word = GP4_FIELD(uint16_t, stream, 8);
    c.eax = (c.eax & 0xffff0000u) | word;
    c.esi = stream + 0xau;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(stream, 0xau));
    GP4_GLOBAL(uint8_t, 0x00874b58) = static_cast<uint8_t>(word);
    GP4_GLOBAL(uint8_t, 0x00874b59) = static_cast<uint8_t>(word >> 8);
    GP4_GLOBAL(uint8_t, 0x00874b3e) = 0x80u;

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
GP4_IMPL(0x00557023, FUN_00557023, "regs(eax:u32, ecx:u32{0xa,0x8,0x108,0x10,0x0}, edx:u32, esi:ptr[0x10]:u16{=0,1,2,3,4,5}, ebx:rec[0x8943a0,0x160,0..25], edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=mode_0086857c+ring_008943a0+bits_0055616f+flags_00874b3e+pan_0055933e+walk_00557083")
