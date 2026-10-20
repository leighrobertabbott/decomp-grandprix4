#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `sub left, right` / `cmp left, right`.
static inline uint32_t sub16Flags(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of a 16-bit `add left, right`.
static inline uint32_t add16Flags(uint16_t left, uint16_t right) {
    const uint32_t sum = static_cast<uint32_t>(left) + right;
    const uint16_t result = static_cast<uint16_t>(sum);
    return parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (sum > 0xffffu ? 1u : 0u);
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0042d7de
// Carry-flag predicate on two car records (ebx, esi) and the globals 0x604153 (signed side byte),
// 0x604404, 0x603e70/0x603b52, 0x603ae0, 0x6039ce/0x6039d0. CF = 0 when the signs of the side byte
// and of [esi+0x90]-[ebx+0x90] agree or when the products compare "less" (signed), else CF = 1.
// eax, ecx and edx keep the intermediate values of the original; the flags are those of the last
// flag-writing instruction on the taken path with CF forced by stc/clc.
void __cdecl FUN_0042d7de(gp4::Regs* r) {
    uint8_t* ebx = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* esi = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t ecxIn = r->ecx;
    const uint32_t edxIn = r->edx;

    const uint8_t sideByte = GP4_GLOBAL(uint8_t, 0x00604153);
    const uint32_t side = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(sideByte)));

    const uint16_t diff90 = static_cast<uint16_t>(GP4_FIELD(const uint16_t, esi, 0x90) - GP4_FIELD(const uint16_t, ebx, 0x90));
    const uint16_t sameSide = static_cast<uint16_t>(static_cast<uint16_t>(side) ^ diff90);
    uint32_t eax = (side & 0xffff0000u) | sameSide;
    uint32_t edx = (edxIn & 0xffff0000u) | diff90;
    uint32_t ecx = ecxIn;
    uint32_t flags;
    bool carry;

    if ((sameSide & 0x8000u) == 0u) {
        // xor ax,dx: SF/ZF/PF of the result, CF = OF = AF = 0
        flags = parityFlag8(sameSide) | (sameSide == 0u ? 0x40u : 0u);
        carry = false;
    } else {
        uint16_t cx = GP4_GLOBAL(uint16_t, 0x00604404);
        if ((sideByte & 0x80u) == 0u) cx = static_cast<uint16_t>(0u - cx);
        ecx = (ecxIn & 0xffff0000u) | cx;

        const uint16_t lhs = GP4_GLOBAL(uint16_t, 0x00603e70);
        const uint16_t rhs = GP4_GLOBAL(uint16_t, 0x00603b52);
        const uint16_t ax = static_cast<uint16_t>(lhs - rhs);
        eax = (side & 0xffff0000u) | ax;
        flags = sub16Flags(lhs, rhs);
        if ((ax & 0x8000u) != 0u) {
            carry = true;
        } else {
            const uint16_t addend = GP4_GLOBAL(uint16_t, 0x00603ae0);
            const uint16_t sumCx = static_cast<uint16_t>(cx + addend);
            flags = add16Flags(cx, addend);
            cx = sumCx;
            ecx = (ecxIn & 0xffff0000u) | cx;
            if ((cx & 0x8000u) != 0u) {
                carry = false;
            } else {
                uint16_t factor = GP4_GLOBAL(uint16_t, 0x006039ce);
                if ((GP4_FIELD(const uint8_t, esi, 0x179) & 0x80u) != 0u) {
                    const uint16_t mix = static_cast<uint16_t>(static_cast<uint16_t>(side) ^ GP4_FIELD(const uint16_t, esi, 0x184));
                    factor = ((mix & 0x8000u) != 0u) ? GP4_GLOBAL(uint16_t, 0x006039ce) : GP4_GLOBAL(uint16_t, 0x006039d0);
                }
                const int32_t first = static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(static_cast<int16_t>(factor));
                const uint16_t diff9a = static_cast<uint16_t>(GP4_FIELD(const uint16_t, esi, 0x9a) - GP4_FIELD(const uint16_t, ebx, 0x9a));
                const int32_t second = static_cast<int32_t>(static_cast<int16_t>(diff9a)) * static_cast<int32_t>(static_cast<int16_t>(cx));
                const uint32_t doubled = static_cast<uint32_t>(first) << 1;
                eax = doubled;
                edx = static_cast<uint32_t>(second);
                flags = cmp32Flags(static_cast<uint32_t>(second), doubled);
                // jl (SF != OF) -> clc, otherwise fall through to stc
                const bool less = (((flags >> 7) ^ (flags >> 11)) & 1u) != 0u;
                carry = !less;
            }
        }
    }

    flags = (flags & ~1u) | (carry ? 1u : 0u);
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042d7de, FUN_0042d7de, "regs(ecx:u32, edx:u32, ebx:ptr[0xa2]:bytes, esi:ptr[0x18c]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
