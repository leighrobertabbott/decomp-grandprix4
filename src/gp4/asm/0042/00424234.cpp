#include <gp4/gp4.h>

// FUNCTION: GP4 0x00424234
void __cdecl FUN_00424234(gp4::Regs* r) {
    // Copies the 12-dword record at esi into the staged globals at 0x00638030..0x00638054 and
    // 0x00623eb8..0x00623ec0 (three of the fields are scaled by 256), rearranging the field order.
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint32_t, 0x00638040) = GP4_FIELD(uint32_t, source, 0x00);
    GP4_GLOBAL(uint32_t, 0x00638044) = GP4_FIELD(uint32_t, source, 0x04);
    GP4_GLOBAL(uint32_t, 0x00638048) = GP4_FIELD(uint32_t, source, 0x08);
    GP4_GLOBAL(uint32_t, 0x0063804c) = GP4_FIELD(uint32_t, source, 0x18) << 8;
    GP4_GLOBAL(uint32_t, 0x00638050) = GP4_FIELD(uint32_t, source, 0x1c) << 8;
    const uint32_t lastScaled = GP4_FIELD(uint32_t, source, 0x20);
    const uint32_t shifted = lastScaled << 8;
    GP4_GLOBAL(uint32_t, 0x00638054) = shifted;
    GP4_GLOBAL(uint32_t, 0x00638034) = GP4_FIELD(uint32_t, source, 0x0c);
    GP4_GLOBAL(uint32_t, 0x00638030) = GP4_FIELD(uint32_t, source, 0x10);
    GP4_GLOBAL(uint32_t, 0x00638038) = GP4_FIELD(uint32_t, source, 0x14);
    GP4_GLOBAL(uint32_t, 0x00623eb8) = GP4_FIELD(uint32_t, source, 0x24);
    GP4_GLOBAL(uint32_t, 0x00623ebc) = GP4_FIELD(uint32_t, source, 0x28);
    const uint32_t last = GP4_FIELD(uint32_t, source, 0x2c);
    GP4_GLOBAL(uint32_t, 0x00623ec0) = last;
    r->eax = last;
    // The last flag writer is the `shl eax, 8` of the field at +0x20: CF = bit 24 shifted out,
    // OF = bit31((value << 7) ^ result), AF = 0, PF = 1 (the low byte of the result is zero).
    const uint32_t flags = ((lastScaled >> 24) & 1u)
        | 0x4u
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((lastScaled << 7) ^ shifted) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424234, FUN_00424234, "regs(esi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
