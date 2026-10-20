#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right` whose result is `result` (32 bit).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// If the dword at +0x4c differs from the dword table entry (0x006098c0, indexed by the byte at
// +0x6d) runs 0x0042453d and then 0x00424553 with ebx = that index. ebx is saved and restored;
// eax = the dword at +0x4c; the flags are those of the compare or of the last callee.
// FUNCTION: GP4 0x0042456e
void __cdecl FUN_0042456e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEbx = r->ebx;
    const uint32_t index = GP4_FIELD(uint8_t, record, 0x6d);
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x4c);
    const uint32_t expected = GP4_ARRAY(uint32_t, 0x006098c0)[index];

    r->ebx = index;
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(value, expected, value - expected);
    if (value != expected) {
        gp4::call_regs(0x0042453d, *r);
        gp4::call_regs(0x00424553, *r);
    }
    r->ebx = savedEbx;
}
GP4_IMPL(0x0042456e, FUN_0042456e, "regs(esi:ptr[0x75]:layout=rec_0042453d, ebx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0042458b+blk_00424553+tag_0042456e")
