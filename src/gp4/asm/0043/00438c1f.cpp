#include <gp4/gp4.h>

// Flags of the 32-bit "cmp left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Signed maximum of the dwords at +0x2c, +0x30 and +0x34 of a 0x5d8-byte entry.
static inline int32_t entryMax(uint8_t* entry) {
    int32_t best = GP4_FIELD(int32_t, entry, 0x2c);
    if (!(best >= GP4_FIELD(int32_t, entry, 0x30))) {
        best = GP4_FIELD(int32_t, entry, 0x30);
    }
    if (!(best >= GP4_FIELD(int32_t, entry, 0x34))) {
        best = GP4_FIELD(int32_t, entry, 0x34);
    }
    return best;
}

// Orders the two records esi and ebx (byte +0x6d indexes the 0x5d8-byte entries at 0x006099c0) by the
// signed maximum of their entry's dwords +0x2c/+0x30/+0x34: [0x607abc] gets the one with the larger
// maximum (esi on a tie), [0x607ac0] the other. Then runs 0x00402225 (angles -> rotation matrix) on
// the first into the block 0x00623038 and on the second into 0x00623108. pushal/popal restore every
// register; the flags are those left by the second call (the `cmp` before it).
// FUNCTION: GP4 0x00438c1f
void __cdecl FUN_00438c1f(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(r->esi);
    uint8_t* second = reinterpret_cast<uint8_t*>(r->ebx);
    uint8_t* table = GP4_ARRAY(uint8_t, 0x006099c0);

    uint8_t* entryFirst = table + static_cast<uint32_t>(first[0x6d]) * 0x5d8u;
    uint8_t* entrySecond = table + static_cast<uint32_t>(second[0x6d]) * 0x5d8u;
    const int32_t maxFirst = entryMax(entryFirst);
    const int32_t maxSecond = entryMax(entrySecond);
    const uint32_t compareFlags = subFlags(static_cast<uint32_t>(maxFirst), static_cast<uint32_t>(maxSecond));

    if (maxFirst < maxSecond) {
        uint8_t* swap = first;
        first = second;
        second = swap;
    }
    GP4_GLOBAL(uint32_t, 0x00607abc) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
    GP4_GLOBAL(uint32_t, 0x00607ac0) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = (r->eflags & ~0x8d5u) | compareFlags;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second));
    c.edx = 0x00623038u;
    gp4::call_regs(0x00402225, c);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second));
    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
    c.edx = 0x00623108u;
    gp4::call_regs(0x00402225, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00438c1f, FUN_00438c1f, "regs(ebx:ptr[0x75]:bytes, esi:ptr[0x75]:bytes) -> (cf, pf, af, zf, sf, of)")
