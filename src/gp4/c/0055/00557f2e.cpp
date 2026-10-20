#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal ... popal routine over the ring record edi (0x160-byte records, ring start [0x866ec0],
// ring length [0x866ebc]). For each of 10 slots k with a nonzero word counter [0x874a98 + 2k]:
// word [0x875d78 + 2k] is or-ed into record word +0x78, and the bits m = (record +0x2c | previous
// record +0x2c) & dword [0x875d50 + 4k] are computed (previous = edi - 0x160, wrapped by + length
// when it drops below the ring start). Four word counter rows (0x874a98, 0x874aac, 0x874ac0,
// 0x874ae8; each 10 words) count down; a row reaching zero ors m into the record dwords
// +0x30 / +0x34,+0x30 / +0x38,+0x34,+0x30 / +0x3c and reloads from 0x874a34, 0x874a48, 0x874a5c,
// 0x874a84. Registers are restored by popal; the flags are those of the final `dec ecx` (1 -> 0),
// with CF = 0 from the last test/or.
// FUNCTION: GP4 0x00557f2e
void __cdecl FUN_00557f2e(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint16_t* const count0 = &GP4_ARRAY(uint16_t, 0x00874a98)[0];
    uint16_t* const count1 = &GP4_ARRAY(uint16_t, 0x00874aac)[0];
    uint16_t* const count2 = &GP4_ARRAY(uint16_t, 0x00874ac0)[0];
    uint16_t* const count4 = &GP4_ARRAY(uint16_t, 0x00874ae8)[0];
    const uint16_t* const reload0 = &GP4_ARRAY(uint16_t, 0x00874a34)[0];
    const uint16_t* const reload1 = &GP4_ARRAY(uint16_t, 0x00874a48)[0];
    const uint16_t* const reload2 = &GP4_ARRAY(uint16_t, 0x00874a5c)[0];
    const uint16_t* const reload4 = &GP4_ARRAY(uint16_t, 0x00874a84)[0];
    const uint16_t* const wordMask = &GP4_ARRAY(uint16_t, 0x00875d78)[0];
    const uint32_t* const bitMask = &GP4_ARRAY(uint32_t, 0x00875d50)[0];

    for (int k = 0; k < 10; ++k) {
        if (count0[k] == 0) {
            continue;
        }
        GP4_FIELD(uint16_t, record, 0x78) |= wordMask[k];
        const uint32_t own = GP4_FIELD(uint32_t, record, 0x2c);
        uint32_t previous = record - 0x160u;
        if (previous < GP4_GLOBAL(uint32_t, 0x00866ec0)) {
            previous += GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        const uint32_t m = (own | GP4_FIELD(uint32_t, previous, 0x2c)) & bitMask[k];
        if (count0[k] != 0) {
            if (--count0[k] == 0) {
                GP4_FIELD(uint32_t, record, 0x30) |= m;
                count0[k] = reload0[k];
            }
        }
        if (count1[k] != 0) {
            if (--count1[k] == 0) {
                GP4_FIELD(uint32_t, record, 0x34) |= m;
                GP4_FIELD(uint32_t, record, 0x30) |= m;
                count1[k] = reload1[k];
            }
        }
        if (count2[k] != 0) {
            if (--count2[k] == 0) {
                GP4_FIELD(uint32_t, record, 0x38) |= m;
                GP4_FIELD(uint32_t, record, 0x34) |= m;
                GP4_FIELD(uint32_t, record, 0x30) |= m;
                count2[k] = reload2[k];
            }
        }
        if (count4[k] != 0) {
            if (--count4[k] == 0) {
                GP4_FIELD(uint32_t, record, 0x3c) |= m;
                count4[k] = reload4[k];
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x00557f2e, FUN_00557f2e, "regs(edi:rec[0x8943a0,0x160,0..29]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+bits_0055616f+flags2c_0055774b+ctr_00557f2e")
