#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Finds the car record (of the 22 at 0x0066fef0, stride 0x41c) whose number (byte +0x7c & 0x3f) equals al & 0x3f.
// esi is the record, or 0 when none matches. ecx is the number of records left (0x16 - index of the match, 0 for
// no match); ah ends as the last compared number. Flags are those of the matching cmp, or of the last
// `add esi, 0x41c` when the search runs out.
// FUNCTION: GP4 0x00445267
void __cdecl FUN_00445267(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax & 0x3fu);
    const uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t count = 0x16u;
    uint32_t found = 0u;
    uint8_t number = 0u;
    uint32_t flags = 0u;

    for (;;) {
        number = static_cast<uint8_t>(car[0x7c] & 0x3fu);
        flags = cmpFlags8(key, number);
        if (key == number) {
            found = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            break;
        }
        flags = addFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
        car += 0x41c;
        --count;
        if (count == 0u) {
            break;
        }
    }

    r->eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(number) << 8) | key;
    r->ecx = count;
    r->esi = found;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445267, FUN_00445267, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=carno_0066ff6c")
