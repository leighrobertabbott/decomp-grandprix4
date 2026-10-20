#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Records reached through ebx = 0x006380d8: walk the pointer instead of using absolute
// addresses for each far field.
static __declspec(noinline) uint32_t negatedPairSum(const uint8_t* ebx) {
    const uint32_t first = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0x634);
    const uint32_t second = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0x934);
    return 0u - (first + second);
}

static __declspec(noinline) uint32_t offsetPairSum(const uint8_t* ebx) {
    const uint32_t first = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0x874);
    const uint32_t second = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0xb74);
    return first + second;
}

// FUNCTION: GP4 0x0041a265
void __cdecl FUN_0041a265(gp4::Regs* r) {
    const uint8_t* owner = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t test = owner[0xcf] & 8u;
    if (test == 0u) {
        // test byte [esi+0xcf], 8 ; je: CF = OF = AF = SF = 0, ZF = 1, PF = 1 (even parity of 0)
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    const uint8_t* ebx = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x006380d8u));

    // neg eax ; imul [0x6013b0] ; shrd eax, edx, 0xe ; + two more words
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(negatedPairSum(ebx)))
                          * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006013b0));
    const uint32_t sum = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 0xe) + offsetPairSum(ebx);
    GP4_GLOBAL(uint32_t, 0x006036e8) = sum;

    // imul [0x603720] ; shrd eax, edx, 0xe ; mov [0x603744], eax ; add [0x603750], eax
    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(sum))
                           * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603720));
    const uint32_t delta = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 0xe);
    GP4_GLOBAL(uint32_t, 0x00603744) = delta;
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00603750);
    GP4_GLOBAL(uint32_t, 0x00603750) = before + delta;

    r->ebx = 0x006380d8u;
    r->eax = delta;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(before, delta);
}
GP4_IMPL(0x0041a265, FUN_0041a265, "regs(esi:ptr[0xd7]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
