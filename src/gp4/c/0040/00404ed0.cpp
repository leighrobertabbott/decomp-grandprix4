#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// `or eax, eax` with eax == 0: CF=OF=AF=0, PF=ZF=1, SF=0.
static inline uint32_t zeroLogicFlags() {
    return 0x44u;
}

// `neg eax` is `0 - eax`: AF comes from (0 ^ value ^ result).
static inline uint32_t negFlags32(uint32_t value, uint32_t result) {
    return (value != 0u ? 1u : 0u)
        | parityFlag8(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | ((value == 0x80000000u) ? 0x800u : 0u);
}

// `sub left, right` on 16 bits.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// `test byte, imm8` with a single-bit mask: CF=OF=AF=0, PF/ZF/SF from the result.
static inline uint32_t testFlags8(uint8_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00404ed0
void __cdecl FUN_00404ed0(gp4::Regs* r) {
    uint8_t* obj = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00602d84));
    uint32_t result = 0;
    uint32_t flags = zeroLogicFlags();
    uint32_t edx = r->edx;
    uint32_t edi = r->edi;

    if (obj != nullptr) {
        edi = reinterpret_cast<uint32_t>(obj);
        const int32_t angle = static_cast<int16_t>(GP4_FIELD(uint16_t, obj, 0x26));
        if (angle != 0) {
            uint16_t ax = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(angle));
            if (static_cast<int16_t>(ax) < 0) {
                ax = static_cast<uint16_t>(-ax);
            }
            ax = static_cast<uint16_t>(ax >> 2);
            const uint32_t offset = ax & 0xfffeu;
            result = static_cast<uint32_t>(static_cast<int32_t>(GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1]));
            const uint8_t statusFlags = GP4_FIELD(uint8_t, obj, 0x51);
            bool negate;
            if ((statusFlags & 0x20u) == 0u) {
                const uint16_t sum = static_cast<uint16_t>(GP4_FIELD(uint16_t, obj, 0x178) + GP4_FIELD(uint16_t, obj, 0x160));
                const uint16_t diff = static_cast<uint16_t>(sum - GP4_FIELD(uint16_t, obj, 0x18));
                const uint16_t lhs = diff;
                const uint16_t rhs = GP4_FIELD(uint16_t, obj, 0x00);
                const uint16_t fin = static_cast<uint16_t>(lhs - rhs);
                edx = (edx & 0xffff0000u) | fin;
                flags = subFlags16(lhs, rhs);
                negate = (fin & 0x8000u) != 0u;
            } else {
                const uint8_t bit = static_cast<uint8_t>(statusFlags & 8u);
                flags = testFlags8(bit);
                negate = bit != 0u;
            }
            if (negate) {
                const uint32_t before = result;
                result = 0u - before;
                flags = negFlags32(before, result);
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x006030b8) = result;
    r->eax = result;
    r->edx = edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00404ed0, FUN_00404ed0, "regs(edi:u32, edx:u32) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_00404ed0")
