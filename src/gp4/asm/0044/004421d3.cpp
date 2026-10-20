#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Stores the bit mask ebp as dword `category` (eax) of the mask table behind pointer global 0x0062ff64.
// For categories 1 and 2 it then walks the set bits of the mask from bit 0 upwards: each set bit i consumes
// one byte from the stream at edi and stores it in byte 0 (category 1) or byte 1 (category 2) of dword i of
// the entry table behind pointer global 0x0062ff68. The loop always runs at least once and ends with
// `or ebp, ebp` on a zero ebp, so the flags are ZF|PF (the cmp eax,2 flags for any other category).
// FUNCTION: GP4 0x004421d3
void __cdecl FUN_004421d3(gp4::Regs* r) {
    const uint32_t category = r->eax;
    uint32_t mask = r->ebp;
    const uint8_t* stream = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t* masks = GP4_GLOBAL(uint32_t*, 0x0062ff64);

    masks[category] = mask;
    uint32_t table = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(masks));

    if (category != 1u && category != 2u) {
        r->ebx = table;
        r->eflags = (r->eflags & ~0x8d5u) | subtractFlags32(category, 2u);
        return;
    }

    uint32_t index = 0;
    uint8_t last = static_cast<uint8_t>(r->edx);
    do {
        const bool take = (mask & 1u) != 0u;
        mask >>= 1;
        if (take) {
            uint8_t* entry = GP4_GLOBAL(uint8_t*, 0x0062ff68) + index * 4u;
            table = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
            last = *stream++;
            entry[category == 1u ? 0 : 1] = last;
        }
        ++index;
    } while (mask != 0u);

    r->ebx = table;
    r->ecx = index;
    r->edx = (r->edx & 0xffffff00u) | last;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(stream));
    r->ebp = 0;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004421d3, FUN_004421d3, "regs(eax:u32[0..8], edx:u32, ebp:u32, edi:ptr[0x40]:bytes) -> (ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00433ff8")
