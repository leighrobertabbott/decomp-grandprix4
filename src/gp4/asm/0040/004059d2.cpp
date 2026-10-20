#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x004059d2
void __cdecl FUN_004059d2(gp4::Regs* r) {
    const uint32_t current = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(current));
    uint32_t flags;
    if (current >= limit) {
        // jae taken: the flags of cmp edi, limit (CF = 0), then clc; edi follows the link
        const uint32_t result = current - limit;
        flags = parityFlag(result)
            | (((current ^ limit ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (((current ^ limit) & (current ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
        r->edi = GP4_FIELD(uint32_t, record, 0x44);
    } else {
        const uint32_t next = GP4_FIELD(uint32_t, record, 0x44);
        // cmp dword [edi+0x44], 0: CF = AF = OF = 0, ZF/SF/PF from the link value
        flags = parityFlag(next)
            | (next == 0u ? 0x40u : 0u)
            | ((next & 0x80000000u) != 0u ? 0x80u : 0u);
        if (next == 0u) {
            flags |= 1u;        // stc: no link, edi unchanged
        } else {
            r->edi = next;      // clc: follow the link
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004059d2, FUN_004059d2, "regs(edi:ptr[0x4c]:layout=link_004059d2) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
