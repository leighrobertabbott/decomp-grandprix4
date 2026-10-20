#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit "test byte, mask" whose result is `result`: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit "cmp left, right".
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Classifies the record in esi into eax: 5 when bit 0x80 of byte +0x7a is set; otherwise by the
// word at +0xea: -1 / -2 for -8 / other negative, 1 / 2 / 3 / 4 for 8 / 0x10 / 0x18 / other positive,
// and for 0 by byte +0xfe (bit 0x40 set) and the sign of the word at +0x94 (+1 or -1 depending on bit
// 0x10). edx is saved and restored; the flags are those of the last test/cmp executed.
// FUNCTION: GP4 0x0042df3a
void __cdecl FUN_0042df3a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(r->esi);
    uint32_t eax;
    uint32_t flags;

    const uint8_t status = record[0x7a];
    if ((status & 0x80u) != 0u) {
        eax = 5u;
        flags = testFlags8(status & 0x80u);
    } else {
        const uint16_t mode = GP4_FIELD(uint16_t, record, 0xea);
        if (mode == 0u) {
            eax = 0u;
            const uint8_t bits = record[0xfe];
            flags = testFlags8(bits & 0x40u);
            if ((bits & 0x40u) != 0u) {
                flags = testFlags8(bits & 0x10u);
                const uint16_t word = GP4_FIELD(uint16_t, record, 0x94);
                flags = cmpFlags16(word, 0u);
                if ((bits & 0x10u) != 0u) {
                    if (static_cast<int16_t>(word) >= 0) {
                        eax = 0xffffffffu;
                    }
                } else {
                    if (static_cast<int16_t>(word) < 0) {
                        eax = 1u;
                    }
                }
            }
        } else if (static_cast<int16_t>(mode) < 0) {
            eax = 0xffffffffu;
            flags = cmpFlags16(mode, 0xfff8u);
            if (mode != 0xfff8u) {
                eax = 0xfffffffeu;
            }
        } else {
            eax = 1u;
            flags = cmpFlags16(mode, 8u);
            if (mode != 8u) {
                eax = 2u;
                flags = cmpFlags16(mode, 0x10u);
                if (mode != 0x10u) {
                    eax = 3u;
                    flags = cmpFlags16(mode, 0x18u);
                    if (mode != 0x18u) {
                        eax = 4u;
                    }
                }
            }
        }
    }

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042df3a, FUN_0042df3a, "regs(esi:ptr[0x106]:layout=lane_0042df3a) -> (eax, cf, pf, af, zf, sf, of)")
