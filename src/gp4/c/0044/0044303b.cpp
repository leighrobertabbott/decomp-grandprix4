#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Walks the 22 car records at 0x0066fef0 (stride 0x41c). A car without bit 7 of +0x7a is counted (ebx, returned
// in ax); when both bit 6 of +0x8d and bit 6 of +0xff are set and its due time +0xf0 is nonzero and already
// reached by the clock [0x0066d814], it is retired through 0x00408742 instead of being counted. Otherwise a car
// without bit 1 of +0x8d is also counted into the word [0x0066d86a]. When that word differs from the previous
// value [0x0066d874] the flag byte [0x00852d70] becomes 0x80; the previous value is then updated. ebx, ecx and
// esi are saved/restored; ax = the count; the flags are those of the final `cmp cx, [0x0066d874]`.
// FUNCTION: GP4 0x0044303b
void __cdecl FUN_0044303b(gp4::Regs* r) {
    uint32_t counted = 0;
    GP4_GLOBAL(uint16_t, 0x0066d86a) = 0;

    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (uint32_t remaining = 0x16; remaining != 0u; --remaining) {
        if ((car[0x7a] & 0x80u) == 0u) {
            bool retired = false;
            if ((car[0x8d] & 0x40u) != 0u && (car[0xff] & 0x40u) != 0u) {
                const uint32_t due = GP4_FIELD(uint32_t, car, 0xf0);
                if (due != 0u && static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - due) >= 0) {
                    gp4::Regs callee = *r;
                    callee.st_in = 0;
                    callee.st_out = 0;
                    callee.ecx = remaining;
                    callee.ebx = counted;
                    callee.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
                    gp4::call_regs(0x00408742, callee);
                    retired = true;
                }
            } else if ((car[0x8d] & 2u) == 0u) {
                GP4_GLOBAL(uint16_t, 0x0066d86a) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0066d86a) + 1u);
            }
            if (!retired) {
                counted += 1u;
            }
        }
        car += 0x41c;
    }

    const uint32_t present = GP4_GLOBAL(uint16_t, 0x0066d86a);
    const uint32_t previous = GP4_GLOBAL(uint16_t, 0x0066d874);
    if (present != previous) {
        GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80;
    }
    GP4_GLOBAL(uint16_t, 0x0066d874) = static_cast<uint16_t>(present);

    r->eax = (r->eax & 0xffff0000u) | (counted & 0xffffu);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags16(present, previous);
}
GP4_IMPL(0x0044303b, FUN_0044303b, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=flags_00408f7b+cars_004436f9+cars_0044303b")
