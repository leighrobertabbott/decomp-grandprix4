#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic op / test on a byte: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint8_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a logic op / test on a word: CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint16_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0042d883
// edi = the record whose byte +0x10 is a signed value and +0x50 a 7-bit amount.
// Selects (from the two flag bytes 0x00604151 & 0x00604152) the value stored at
// 0x00604154: the byte +0x10 itself, or with the top bit flipped. CF set = nothing selected.
void __cdecl FUN_0042d883(gp4::Regs* r) {
    uint8_t* const body = reinterpret_cast<uint8_t*>(r->edi);
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t eax = r->eax;
    uint32_t flags;
    bool flip;   // true: store byte ^ 0x80

    const uint8_t sel = GP4_GLOBAL(uint8_t, 0x00604151) & GP4_GLOBAL(uint8_t, 0x00604152);
    if (sel == 0) {
        // or dl,dl ; stc
        edx &= ~0xffu;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(0) | 1u;
        return;
    }

    if (sel == 0x80) {
        flags = cmpFlags16(0, 0);   // cmp dl,0x80 equal: ZF | PF
        flip = false;
    } else if (sel == 0x40) {
        flip = true;
        flags = 0;
    } else {
        const uint8_t sign = body[0x10];
        uint16_t cx = GP4_GLOBAL(uint16_t, 0x00604404);
        if ((sign & 0x80u) != 0u) {
            cx = static_cast<uint16_t>(cx + 0x200);
        } else {
            cx = static_cast<uint16_t>(static_cast<uint16_t>(-cx) + 0x200);
        }
        flags = logicFlags16(cx);   // or cx,cx
        if ((cx & 0x8000u) != 0u) {
            ecx = (ecx & 0xffff0000u) | cx;
            flip = false;
        } else {
            if (static_cast<int16_t>(cx) > 0x400) {
                cx = 0x400;
            }
            cx = static_cast<uint16_t>(0x400 - cx);
            GP4_GLOBAL(uint16_t, 0x006077cc) = cx;
            ecx = (ecx & 0xffff0000u) | cx;

            gp4::Regs callee = *r;
            gp4::call_regs(0x00401462, callee);
            const uint32_t seed = callee.eax & 0xffu;
            const uint32_t amount = static_cast<uint32_t>(body[0x50]) & 0x7fu;
            const uint32_t product = (seed << 2) * amount;
            uint32_t divisor = static_cast<uint32_t>(body[0x10]) & 0x7fu;
            if (divisor == 0) {
                divisor = 1;
            }
            const uint32_t quotient = product / divisor;
            const uint32_t remainder = product % divisor;
            eax = quotient;
            ecx = divisor;
            edx = remainder;
            const uint16_t threshold = GP4_GLOBAL(uint16_t, 0x006077cc);
            flags = cmpFlags16(static_cast<uint16_t>(quotient), threshold);
            flip = !(static_cast<int16_t>(quotient) < static_cast<int16_t>(threshold));
        }
    }

    uint8_t value = body[0x10];
    if (flip) {
        value ^= 0x80;
        flags = logicFlags8(value);
    }
    edx = (edx & ~0xffu) | value;
    GP4_GLOBAL(uint8_t, 0x00604154) = value;

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u);   // clc
}
GP4_IMPL(0x0042d883, FUN_0042d883, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[0x58]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
