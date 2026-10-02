#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags left by a byte-wide OR/AND/TEST: CF=OF=AF=0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t flags = parityFlag(result);
    if ((result & 0xffu) == 0u) flags |= 0x040u;
    if ((result & 0x80u) != 0u) flags |= 0x080u;
    return flags;
}

// FUNCTION: GP4 0x00434e1e
void __cdecl FUN_00434e1e(gp4::Regs* r) {
    const uint8_t selector = static_cast<uint8_t>(r->eax);
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    uint8_t status = GP4_FIELD(uint8_t, record, 0x163);
    status |= 0x80;
    GP4_FIELD(uint8_t, record, 0xce) |= 0x04;
    GP4_FIELD(uint8_t, record, 0xff) &= 0xdf;
    GP4_FIELD(uint8_t, record, 0x122) &= 0x7f;
    status |= 0x40;
    status &= 0xdf;

    // `or al, al` sets the flags; when al is negative the final `or` into
    // the status byte replaces them.
    uint32_t flags = logicFlags8(selector);
    if ((selector & 0x80u) != 0u) {
        status |= 0x20;
        flags = logicFlags8(status);
    }
    GP4_FIELD(uint8_t, record, 0x163) = status;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434e1e, FUN_00434e1e, "regs(eax:u32, esi:ptr[0x16b]:bytes) -> (cf, pf, af, zf, sf, of)")
