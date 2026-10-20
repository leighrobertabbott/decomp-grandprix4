#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Scales the byte at ebx[ecx + 0x28] by DAT_00601d5c (>> 15 through the 64-bit product) into the
// dword slot esi[0x2dc + ecx*4]; the result is multiplied by 8 when bit 0 of the byte at
// esi+0x8d is set. eax and edx (high half of the product) are left changed. The flags are those
// of the final shl, or of the "test byte ptr [esi+0x8d], 1" when the shl is skipped.
// FUNCTION: GP4 0x004100d8
void __cdecl FUN_004100d8(gp4::Regs* r) {
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = r->ecx;

    // mov al, [ecx+ebx+0x28] ; and eax, 0xff ; imul dword [DAT_00601d5c] -> edx:eax
    const int32_t sample = static_cast<int32_t>(source[index + 0x28u]);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601d5c);
    const int64_t product = static_cast<int64_t>(sample) * static_cast<int64_t>(scale);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    // shrd eax, edx, 15
    uint32_t result = (low >> 15) | (high << 17);
    uint32_t flags;

    if ((dest[0x8d] & 1u) != 0u) {
        // shl eax, 3: CF = last bit out, AF = 0, OF = bit31(result << 2 ^ result << 3)
        const uint32_t shifted = result << 3;
        flags = ((result >> 29) & 1u)
            | parityFlag(shifted)
            | (shifted == 0u ? 0x40u : 0u)
            | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
            | ((((result << 2) ^ shifted) & 0x80000000u) != 0u ? 0x800u : 0u);
        result = shifted;
    } else {
        // test byte, 1 with a zero result: CF = OF = AF = SF = 0, ZF = PF = 1
        flags = 0x44u;
    }

    GP4_FIELD(uint32_t, dest, index * 4u + 0x2dcu) = result;

    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004100d8, FUN_004100d8, "regs(ebx:ptr[0x40]:bytes, ecx:u32[0..15], esi:ptr[0x320]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
