#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// or reg,reg / test: CF = OF = AF = 0
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

struct Unknown_0041da34 {
    uint8_t Unknown00[0xfc];
    int16_t UnknownFC;
    int16_t UnknownFE;
    uint8_t Unknown100[0x25c - 0x100];
    int16_t Unknown25C;
    int16_t Unknown25E;
};

// FUNCTION: GP4 0x0041da34
void __cdecl FUN_0041da34(gp4::Regs* r) {
    const Unknown_0041da34* body = reinterpret_cast<const Unknown_0041da34*>(static_cast<uintptr_t>(r->edi));
    const bool selectorIsThree = r->edx == 3u;
    const int32_t input = static_cast<int32_t>(r->eax);
    int32_t value = input;
    int32_t divisor;

    const uint8_t disabled = GP4_GLOBAL(uint8_t, 0x006246e4);
    if (disabled != 0) {
        // cmp byte [0x6246e4], 0 (byte-wide flags: SF is bit 7) ... clc
        r->eflags = (r->eflags & ~0x8d5u) | parityFlag(disabled) | ((disabled & 0x80u) != 0u ? 0x80u : 0u);
        return;
    }

    divisor = selectorIsThree ? body->UnknownFC : body->UnknownFE;
    if (divisor != 0) {
        if (divisor < 0) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(static_cast<uint32_t>(divisor));
            return;
        }
        const uint32_t difference = static_cast<uint32_t>(input) - static_cast<uint32_t>(divisor);
        if (static_cast<int32_t>(difference) < 0) {
            // the failing "sub eax, ebp" has already updated eax
            r->eax = difference;
            r->eflags = (r->eflags & ~0x8d5u)
                | (subtractionFlags32(static_cast<uint32_t>(input), static_cast<uint32_t>(divisor)) & ~1u);
            return;
        }
        value = static_cast<int32_t>(difference);
        divisor = 0x4000 - divisor;
    } else {
        divisor = selectorIsThree ? body->Unknown25C : body->Unknown25E;
        if (divisor >= 0) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(static_cast<uint32_t>(divisor)) | 1u;
            return;
        }
        divisor += 0x4000;
        if (input >= divisor) {
            r->eflags = (r->eflags & ~0x8d5u)
                | (subtractionFlags32(static_cast<uint32_t>(input), static_cast<uint32_t>(divisor)) & ~1u);
            return;
        }
    }

    if (divisor == 0) {
        divisor = 1;
    }
    if (value < 0) {
        value = 0;
    }
    if (value > 0x4000) {
        value = 0x4000;
    }
    int32_t remainder;
    int32_t quotient = gp4::idiv64(static_cast<int64_t>(value) << 14, divisor, &remainder);
    if (quotient < 0) {
        quotient = 0;
    }
    const uint32_t compareFlags = subtractionFlags32(static_cast<uint32_t>(quotient), 0x4000u);
    if (quotient > 0x4000) {
        quotient = 0x4000;
    }
    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | compareFlags | 1u;
}
GP4_IMPL(0x0041da34, FUN_0041da34, "regs(eax:i32[-0x6000..0x6000], edx:u32[0..5], edi:ptr[0x270]:layout=body_0041da34) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041da34")
