#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004129e0
void __cdecl FUN_004129e0(gp4::Regs* r) {
    uint32_t* target = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int32_t>(r->eax);
    // push eax ; imul [0x623ef4] ; sar eax, 6 ; add [esi], eax ; pop eax
    const int64_t firstProduct = static_cast<int64_t>(value) * GP4_GLOBAL(int32_t, 0x00623ef4);
    target[0] += static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(firstProduct)) >> 6);
    // imul [0x623ef8] ; sar eax, 6 ; add [esi+4], eax
    const int64_t secondProduct = static_cast<int64_t>(value) * GP4_GLOBAL(int32_t, 0x00623ef8);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(secondProduct)) >> 6);
    const uint32_t before = target[1];
    const uint32_t result = before + shifted;
    target[1] = result;
    // the flags are those of the final add
    const uint32_t flags = (result < before ? 1u : 0u)
        | parityFlag(result)
        | (((before ^ shifted ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(before ^ shifted) & (before ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = shifted;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(secondProduct) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004129e0, FUN_004129e0, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
