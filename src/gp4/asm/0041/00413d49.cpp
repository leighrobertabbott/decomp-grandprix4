#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 16-bit "cmp left, right" except CF (stc/clc set it afterwards).
static inline uint32_t cmp16FlagsNoCarry(uint16_t left, uint16_t right) {
    const uint32_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Tests whether the two words at esi+0xe and esi+0x18, each measured from the word at edi, lie
// inside [[0x602094], [0x602096]] and are within 0xe00 of each other (all signed 16-bit). The
// bounds are first recomputed from the words at edi+0x60/+0x62 and esi+0x90 scaled by [0x602090]
// (>> 14). CF = 1 when all tests pass (stc), CF = 0 (clc) at the first failure; the other flags are
// those of the last `cmp`, eax keeps the high word of the second bound product, ax the last value.
// FUNCTION: GP4 0x00413d49
void __cdecl FUN_00413d49(gp4::Regs* r) {
    uint8_t* other = reinterpret_cast<uint8_t*>(r->edi);
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);

    const uint32_t ebp = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, body, 0x90)));
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00602090);

    const uint32_t first = 0u - (static_cast<uint32_t>(GP4_FIELD(uint16_t, other, 0x60)) + 0x50u) - ebp;
    const int64_t product1 = static_cast<int64_t>(static_cast<int32_t>(first)) * static_cast<int64_t>(scale);
    const uint32_t shifted1 = static_cast<uint32_t>(static_cast<uint64_t>(product1) >> 14);
    const int16_t lower = static_cast<int16_t>(shifted1);
    GP4_GLOBAL(int16_t, 0x00602094) = lower;

    const uint32_t second = static_cast<uint32_t>(GP4_FIELD(uint16_t, other, 0x62)) + 0x50u - ebp;
    const int64_t product2 = static_cast<int64_t>(static_cast<int32_t>(second)) * static_cast<int64_t>(scale);
    const uint32_t shifted2 = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14);
    const int16_t upper = static_cast<int16_t>(shifted2);
    GP4_GLOBAL(int16_t, 0x00602096) = upper;

    const uint16_t origin = GP4_FIELD(uint16_t, other, 0x00);
    uint16_t ax = 0;
    uint32_t flags = 0;
    bool failed = false;

    for (uint32_t i = 0; i < 2 && !failed; ++i) {
        ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, i == 0 ? 0x0e : 0x18) - origin);
        if (static_cast<int16_t>(ax) < 0) {
            flags = cmp16FlagsNoCarry(ax, static_cast<uint16_t>(lower));
            failed = static_cast<int16_t>(ax) < lower;
        } else {
            flags = cmp16FlagsNoCarry(ax, static_cast<uint16_t>(upper));
            failed = static_cast<int16_t>(ax) > upper;
        }
    }
    if (!failed) {
        ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0x0e) - GP4_FIELD(uint16_t, body, 0x18));
        if (static_cast<int16_t>(ax) < 0) {
            ax = static_cast<uint16_t>(0u - ax);
        }
        flags = cmp16FlagsNoCarry(ax, 0x0e00u);
        failed = static_cast<int16_t>(ax) > 0x0e00;
    }

    r->eax = (shifted2 & 0xffff0000u) | ax;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 32);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags | (failed ? 0u : 1u);
}
GP4_IMPL(0x00413d49, FUN_00413d49, "regs(edi:ptr[0x6a]:bytes, esi:ptr[0x98]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
