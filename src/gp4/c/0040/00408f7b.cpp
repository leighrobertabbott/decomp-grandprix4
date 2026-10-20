#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Walks the 22 records at 0x66fef0 (stride 0x41c); every record whose byte at +0x7c is not
// negative is run through 0x408742 and gets bit 3 set in the byte at +0x8d. pushal/popal restore
// the registers, so only the flags of the final "add esi, 0x41c" survive.
// FUNCTION: GP4 0x00408f7b
void __cdecl FUN_00408f7b(gp4::Regs* r) {
    uint32_t record = 0x0066fef0u;
    uint32_t flags = 0u;

    for (uint32_t count = 0x16u; count != 0u; --count) {
        uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        if (static_cast<int8_t>(body[0x7c]) >= 0) {
            gp4::Regs callee = *r;
            callee.st_in = 0;
            callee.st_out = 0;
            callee.esi = record;
            callee.ecx = count;
            gp4::call_regs(0x00408742, callee);
            body[0x8d] |= 8;
        }
        const uint32_t next = record + 0x41cu;
        flags = addFlags(record, 0x41cu, next);
        record = next;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408f7b, FUN_00408f7b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
