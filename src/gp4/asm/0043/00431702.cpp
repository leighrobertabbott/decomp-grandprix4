#include <gp4/gp4.h>

// Updates the flag bytes of a car record (esi) from the word at +0x90 (made non-positive-signed by
// the NEG pair: |x|), the limit word at +0xb0 and the word at +0x9a.
//  - byte +0xcc bit 0 set: unless word +0x9a >= 0x3ab and |+0x90| >= +0xb0 (a plain return), clear
//    +0xcc bits 0/1 and +0x230 bit 4, set +0x1ce bit 6 and +0xfe bit 3 and store 0xff at +0x174;
//  - byte +0xcc bit 0 clear: set +0xcc bit 0 when |+0x90| >= +0xb0, set +0xcb bit 7 and +0x230 bit 4,
//    clear +0xca bit 0, store 0xff at +0x174 and set +0xca bit 1 when +0xe5 has bit 7.
// Only ax changes (the upper half of eax stays). Flags are those of the last flag writer on each path.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// Flags of a 16-bit CMP a, b.
static inline uint32_t cmp16_flags(uint32_t a, uint32_t b) {
    const uint32_t res = (a - b) & 0xffffu;
    return (a < b ? 1u : 0u)
        | parity_flag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x8000u) ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x8000u) ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (OR/TEST): CF = OF = AF = 0.
static inline uint32_t logic8_flags(uint32_t res) {
    return parity_flag(res) | ((res & 0xffu) == 0u ? 0x40u : 0u) | ((res & 0x80u) ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00431702
void __cdecl FUN_00431702(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint16_t ax;
    uint32_t flags;

    if (GP4_FIELD(uint8_t, rec, 0xcc) & 1u) {
        ax = GP4_FIELD(uint16_t, rec, 0x9a);
        bool clear = true;
        if (static_cast<int16_t>(ax) >= 0x3ab) {
            ax = GP4_FIELD(uint16_t, rec, 0x90);
            if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
            const uint16_t limit = GP4_FIELD(uint16_t, rec, 0xb0);
            if (static_cast<int16_t>(ax) >= static_cast<int16_t>(limit)) {
                flags = cmp16_flags(ax, limit);
                clear = false;
            }
        }
        if (clear) {
            GP4_FIELD(uint8_t, rec, 0xcc) &= 0xfdu;
            GP4_FIELD(uint8_t, rec, 0xcc) &= 0xfeu;
            GP4_FIELD(uint8_t, rec, 0x230) &= 0xefu;
            GP4_FIELD(uint8_t, rec, 0x1ce) |= 0x40u;
            GP4_FIELD(uint16_t, rec, 0x174) = 0xffu;
            GP4_FIELD(uint8_t, rec, 0xfe) |= 8u;
            flags = logic8_flags(GP4_FIELD(uint8_t, rec, 0xfe));
        }
    } else {
        ax = GP4_FIELD(uint16_t, rec, 0x90);
        if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
        const uint16_t limit = GP4_FIELD(uint16_t, rec, 0xb0);
        if (!(static_cast<int16_t>(ax) < static_cast<int16_t>(limit))) {
            GP4_FIELD(uint8_t, rec, 0xcc) |= 1u;
        }
        GP4_FIELD(uint8_t, rec, 0xcb) |= 0x80u;
        GP4_FIELD(uint8_t, rec, 0x230) |= 0x10u;
        GP4_FIELD(uint8_t, rec, 0xca) &= 0xfeu;
        GP4_FIELD(uint16_t, rec, 0x174) = 0xffu;
        if (GP4_FIELD(uint8_t, rec, 0xe5) & 0x80u) {
            GP4_FIELD(uint8_t, rec, 0xca) |= 2u;
            flags = logic8_flags(GP4_FIELD(uint8_t, rec, 0xca));
        } else {
            flags = 0x44u;      // TEST with a zero result: ZF = PF = 1
        }
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431702, FUN_00431702, "regs(eax:u32, esi:ptr[0x238]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
