#include <gp4/gp4.h>

// Only the fields this routine reads (the byte table at +0x1c6 is read by 0x41242f).
struct Unknown_0040f833 {
    uint8_t Unknown000[0x1cd];
    int8_t Unknown1cd;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t compareFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffffu;
    return (sum > 0xffffu ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// ax = a 16-bit target value, esi = record. Tries the slots cl = 1..byte[esi+0x1cd]: each slot's
// 0x41242f ratio times the target (Q14) minus DAT_00603834, absolute value, and keeps the slot
// with the smallest result (the first on ties). DAT_0062fbfc = best slot (or 9 when the target is
// outside [-0x8000, 0x6000) or at least 0x7700). A pushal ... popal wrapper: every register is
// restored; the flags are those of the last flag writer (compare of ax, or the final add ax, 1).
// FUNCTION: GP4 0x0040f833
void __cdecl FUN_0040f833(gp4::Regs* r) {
    const Unknown_0040f833* body = reinterpret_cast<const Unknown_0040f833*>(static_cast<uintptr_t>(r->esi));
    const int16_t target = static_cast<int16_t>(r->eax & 0xffffu);

    if (target >= 0x6000) {
        GP4_GLOBAL(uint32_t, 0x0062fbfc) = 9u;
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags16(static_cast<uint16_t>(target), 0x6000u);
        return;
    }
    if (target >= 0x7700) {
        GP4_GLOBAL(uint32_t, 0x0062fbfc) = 9u;
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags16(static_cast<uint16_t>(target), 0x7700u);
        return;
    }

    const int32_t scale = target;
    int16_t best = 0x7fff;
    uint8_t bestSlot = static_cast<uint8_t>(r->ecx >> 8);
    int8_t slot = 1;
    do {
        gp4::Regs s = *r;
        s.eax = (r->eax & 0xffffff00u) | static_cast<uint8_t>(slot);
        s.edx = r->esi + 0x1c6u;
        gp4::call_regs(0x0041242f, s);

        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(s.eax)) * static_cast<int64_t>(scale);
        uint16_t value = static_cast<uint16_t>(static_cast<uint64_t>(product) >> 14);
        value = static_cast<uint16_t>(value - GP4_GLOBAL(uint16_t, 0x00603834));
        if ((value & 0x8000u) != 0u) value = static_cast<uint16_t>(0u - value);
        if (static_cast<int16_t>(value) <= best) {
            best = static_cast<int16_t>(value);
            bestSlot = static_cast<uint8_t>(slot);
        }
        slot = static_cast<int8_t>(slot + 1);
    } while (slot <= body->Unknown1cd);

    GP4_GLOBAL(uint32_t, 0x0062fbfc) = static_cast<uint32_t>(bestSlot) + 1u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(bestSlot, 1u);
}
GP4_IMPL(0x0040f833, FUN_0040f833, "regs(eax:u32, esi:ptr[0x258]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
