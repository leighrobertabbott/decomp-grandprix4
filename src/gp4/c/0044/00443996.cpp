#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of a 16-bit `and` result (CF = OF = AF = 0).
static inline uint32_t logicFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void finish(gp4::Regs* r, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t flags,
                          bool mark) {
    if (mark) {
        GP4_GLOBAL(uint8_t, 0x0066d87c) = 0x80;
    }
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x00443996
// Per-frame trigger that sets the byte [0x66d87c] to 0x80. With [0x7c2e75] set it fires when the product of the
// words [0x7ad89c] and [0x7c2eb8] equals the word [0x66d894]. Otherwise (and only while the counter [0x66d8cc] is
// at most 0x1d4c0) it first schedules a one-off event for the selected car [0x679d30] through 0x00418fd0 (flag
// byte [0x63b85c]); while [0x66d8cc] is zero and the byte [0x66d87a] is not negative it steps the 4-bit counter
// word [0x66d878] and acts every 16th step: with the mode word [0x7c2eb8] below 2 it prepares the segment ring
// (0x441a78, 0x441c92, byte [0x66d87f] = 0x80) and fires when 0x443955 finds no free car (CF set); otherwise it
// fires when [0x7c2ea9] bit 6 is clear, [0x88f63a] or [0x852d6a] is set, or the word [0x66d894] is not below
// four times [0x7c2eb8] (equal to one of its first four multiples counts). eax, ecx and edx are left as the
// original leaves them (eax = [0x66d8cc] or the product, low words replaced on the multiples check); the flags are
// those of the last compare / test.
void __cdecl FUN_00443996(gp4::Regs* r) {
    uint32_t eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t esi = r->esi;
    uint32_t flags;

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u) {
        const uint32_t product = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007ad89c))
            * static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2eb8));
        eax = product;
        edx = GP4_GLOBAL(uint16_t, 0x0066d894);
        finish(r, eax, ecx, edx, esi, cmpFlags32(eax, edx), eax == edx);
        return;
    }

    eax = GP4_GLOBAL(uint32_t, 0x0066d8cc);
    if (static_cast<int32_t>(eax) > 0x1d4c0) {
        finish(r, eax, ecx, edx, esi, cmpFlags32(eax, 0x1d4c0u), false);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    if (GP4_GLOBAL(uint8_t, 0x0063b85c) == 0u) {
        esi = GP4_GLOBAL(uint32_t, 0x00679d30);
        c.eax = 0;
        c.ecx = 0xc;
        c.edx = 0;
        c.esi = esi;
        gp4::call_regs(0x00418fd0, c);
        GP4_GLOBAL(uint8_t, 0x0063b85c) = 0xff;
    }

    const uint32_t counter = GP4_GLOBAL(uint32_t, 0x0066d8cc);
    if (counter != 0u) {
        finish(r, eax, ecx, edx, esi, testFlags32(counter), false);
        return;
    }
    const uint32_t blocked = GP4_GLOBAL(uint8_t, 0x0066d87a);
    if ((blocked & 0x80u) != 0u) {
        finish(r, eax, ecx, edx, esi, testFlags8(blocked), false);
        return;
    }
    const uint32_t step = (static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0066d878)) + 1u) & 0xfu;
    GP4_GLOBAL(uint16_t, 0x0066d878) = static_cast<uint16_t>(step);
    if (step != 0u) {
        finish(r, eax, ecx, edx, esi, logicFlags16(step), false);
        return;
    }

    const uint32_t mode = GP4_GLOBAL(uint16_t, 0x007c2eb8);
    if (static_cast<int16_t>(mode) < 2) {
        c.eax = eax;
        c.ecx = ecx;
        c.edx = edx;
        c.esi = esi;
        if ((GP4_GLOBAL(uint8_t, 0x0066d87f) & 0x80u) == 0u) {
            gp4::call_regs(0x00441a78, c);
            gp4::call_regs(0x00441c92, c);
            GP4_GLOBAL(uint8_t, 0x0066d87f) = 0x80;
        }
        gp4::call_regs(0x00443955, c);
        finish(r, eax, ecx, edx, esi, c.eflags & 0x8d5u, (c.eflags & 1u) != 0u);
        return;
    }

    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u;
    if (gate == 0u) {
        finish(r, eax, ecx, edx, esi, testFlags8(gate), true);
        return;
    }
    const uint32_t flagA = GP4_GLOBAL(uint8_t, 0x0088f63a);
    if (flagA != 0u) {
        finish(r, eax, ecx, edx, esi, testFlags8(flagA), true);
        return;
    }
    const uint32_t flagB = GP4_GLOBAL(uint8_t, 0x00852d6a);
    if (flagB != 0u) {
        finish(r, eax, ecx, edx, esi, testFlags8(flagB), true);
        return;
    }

    const uint32_t value = GP4_GLOBAL(uint16_t, 0x0066d894);
    const uint32_t unit = GP4_GLOBAL(uint16_t, 0x007c2eb8);
    uint32_t multiple = unit;
    bool mark = false;
    for (uint32_t k = 0;; ++k) {
        flags = cmpFlags16(value, multiple);
        if (k == 3u) {
            mark = !(static_cast<int16_t>(value) < static_cast<int16_t>(multiple));
            break;
        }
        if (value == multiple) {
            mark = true;
            break;
        }
        multiple = (multiple + unit) & 0xffffu;
    }
    finish(r, (eax & 0xffff0000u) | value, (ecx & 0xffff0000u) | multiple, (edx & 0xffff0000u) | unit, esi, flags, mark);
}
GP4_IMPL(0x00443996, FUN_00443996, "regs(ecx:u32, edx:u32, esi:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+cars_00433d8d+flags_00432dcf+state_00432f18+state_004320f2+ring_008943a0+words_008943a0+ptr_0086850c+geom_008943a0+state_0066d8c0+state_0044334d+state_0040eb6b+state_00443996")
