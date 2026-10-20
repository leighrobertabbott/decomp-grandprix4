#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right` (also `neg x` as cmp 0, x).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Looks up the 0x403263 curve at ax (when ax >= 0) and at ax - 0x17a (when that is >= 0), selector
// 3 when the word at esi+0x90 is negative, else 2. eax = (difference * DAT_00602fb8) >> 14,
// negated when the word is negative; edx = |difference| >> 1. ebx, ecx, ebp are saved and restored.
// FUNCTION: GP4 0x00403497
void __cdecl FUN_00403497(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(r->esi);
    const int32_t position = static_cast<int16_t>(r->eax);
    const uint16_t word = *reinterpret_cast<const uint16_t*>(body + 0x90);
    const bool negative = (word & 0x8000u) != 0u;
    const uint32_t selector = negative ? 3u : 2u;

    uint32_t first = 0u;
    uint32_t second = 0u;
    if (position >= 0) {
        gp4::Regs callee = *r;
        callee.ecx = static_cast<uint32_t>(position);
        callee.edx = selector;
        gp4::call_regs(0x00403263, callee);
        first = callee.eax;
    }
    const int32_t shifted = position - 0x17a;
    if (shifted >= 0) {
        gp4::Regs callee = *r;
        callee.ecx = static_cast<uint32_t>(shifted);
        callee.edx = selector;
        gp4::call_regs(0x00403263, callee);
        second = callee.eax;
    }

    const uint32_t difference = second - first;
    const uint32_t magnitude = static_cast<int32_t>(difference) < 0 ? 0u - difference : difference;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(difference))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00602fb8));
    uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    uint32_t flags;
    if (negative) {
        flags = subFlags(0u, scaled);   // neg eax
        scaled = 0u - scaled;
    } else {
        // test word, 0xffff: CF = OF = AF = 0, SF clear (jns taken)
        flags = parityFlag(word) | (word == 0u ? 0x40u : 0u);
    }

    r->eax = scaled;
    r->edx = magnitude >> 1;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403497, FUN_00403497, "regs(eax:u32, esi:ptr[0x94]:bytes, edi:ptr[0x13a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
