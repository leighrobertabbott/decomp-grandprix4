#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint8_t* asBytes(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x00432330
// Tests whether the record linked at [esi+0x1c] is in range: the CF result is clear when
// the 11-bit stamp at +0x1a of the (possibly linked) record, subtracted from the 16-bit
// stamp at 0x00868564, lies in [DAT_0062a950, DAT_0062a954).
void __cdecl FUN_00432330(gp4::Regs* r) {
    uint8_t* body = asBytes(r->esi);
    const uint32_t edx = GP4_GLOBAL(uint16_t, 0x00868564);
    uint32_t eax = GP4_FIELD(uint32_t, body, 0x1c);
    uint32_t flags = cmpFlags32(eax, GP4_GLOBAL(uint32_t, 0x00891500));
    bool carry = true;
    bool done = false;

    if (eax < GP4_GLOBAL(uint32_t, 0x00891500)) {
        flags = cmpFlags32(eax, GP4_GLOBAL(uint32_t, 0x00866f00));
        if (eax < GP4_GLOBAL(uint32_t, 0x00866f00)) {
            done = true;                       // stc, eax unchanged
        } else {
            eax = GP4_FIELD(uint32_t, asBytes(eax), 0x44);
            flags = parityFlag(eax) | (eax == 0u ? 0x40u : 0u) | ((eax & 0x80000000u) != 0u ? 0x80u : 0u);  // or eax, eax
            if (eax == 0u) done = true;        // stc
        }
    }

    if (!done) {
        const uint32_t stamp = GP4_FIELD(uint16_t, asBytes(eax), 0x1a) & 0x7ffu;
        const uint32_t negated = 0u - stamp;
        const uint32_t diff = negated + edx;
        eax = diff;
        flags = cmpFlags32(diff, GP4_GLOBAL(uint32_t, 0x0062a950));
        if (static_cast<int32_t>(diff) >= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062a950))) {
            flags = cmpFlags32(diff, GP4_GLOBAL(uint32_t, 0x0062a954));
            if (static_cast<int32_t>(diff) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062a954))) {
                carry = false;                 // clc
            }
        }
    }

    flags = (flags & ~0x1u) | (carry ? 1u : 0u);
    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432330, FUN_00432330, "regs(esi:u32[0x66fef0..0x66fef0]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00432330")
