#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the car record for index [0x00a69878] (clamped to 0..21 by 0x005106f6, record in esi) and
// stores two results: [0x00a6987c] = 1 when bit 6 of the record byte +0x87 is set, else 0; in the
// set case [0x00a69880] = (((|record +0x98| >> 8) arithmetic >> 8) * 0x2ba) >> 16 (0x0040824b gives
// |+0x98| >> 8). pushal/popal wrapper: every GPR is restored, the flags are those of the last flag
// writer (popal does not restore them): the "test byte [esi+0x87],0x40" when the bit is clear, else
// the final "shr eax,16".
// FUNCTION: GP4 0x00510bb2
void __cdecl FUN_00510bb2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x005106f6, c);
    const uint8_t* record = (const uint8_t*)(uintptr_t)c.esi;

    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    const uint8_t selector = record[0x87] & 0x40u;
    uint32_t arith = gp4::flags::logic8(selector);
    if (selector != 0u) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
        gp4::call_regs(0x0040824b, c);
        const uint32_t scaled = (uint32_t)(((int32_t)c.eax >> 8) * 0x2ba);
        const uint32_t result = scaled >> 16;
        GP4_GLOBAL(uint32_t, 0x00a69880) = result;
        // shr eax,16: CF = last bit shifted out, AF = 0, OF = bit31(scaled>>15 ^ result) = 0
        arith = gp4::flags::szp(result, 32) | ((scaled >> 15) & 1u);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00510bb2, FUN_00510bb2, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0")
