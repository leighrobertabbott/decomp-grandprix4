#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Copies a 0x14-byte source record (ebx) into the destination (edi): the first dword gets
// DAT_006082c4 added, then dwords +4, +8 and +0x10 are copied as is. esi counts up and edi
// advances to the next record; the flags are those of the final "add edi, 0x14".
// FUNCTION: GP4 0x00422991
void __cdecl FUN_00422991(gp4::Regs* r) {
    const uint32_t* source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t* target = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->edi));

    target[0] = source[0] + GP4_GLOBAL(uint32_t, 0x006082c4);
    target[1] = source[1];
    target[2] = source[2];
    const uint32_t last = source[4];
    target[4] = last;

    const uint32_t before = r->edi;
    const uint32_t after = before + 0x14u;
    const uint32_t flags = (after < before ? 0x1u : 0u)
        | parityFlag(after)
        | (((before ^ 0x14u ^ after) & 0x10u) != 0u ? 0x10u : 0u)
        | (after == 0u ? 0x40u : 0u)
        | ((after >> 24) & 0x80u)
        | ((((before ^ after) & (0x14u ^ after)) >> 31) != 0u ? 0x800u : 0u);

    r->eax = last;
    r->esi = r->esi + 1u;
    r->edi = after;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00422991, FUN_00422991, "regs(ebx:ptr[0x20]:bytes, edi:ptr[0x20]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
