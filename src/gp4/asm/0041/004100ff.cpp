#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x004100ff
void __cdecl FUN_004100ff(gp4::Regs* r) {
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = r->ecx;

    // mov al, [ecx+ebx+0x24] ; and eax, 0xff ; imul dword [DAT_00601d68] -> edx:eax
    const int32_t sample = static_cast<int32_t>(source[index + 0x24u]);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601d68);
    const int64_t product = static_cast<int64_t>(sample) * static_cast<int64_t>(scale);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    // shrd eax, edx, 14
    const uint32_t result = (low >> 14) | (high << 18);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x2ccu) = result;

    // Flags of the shrd (pinned oracle model): CF = last bit shifted out,
    // OF = sign change between the count-1 and count shifts, AF = 0.
    const uint32_t previous = (low >> 13) | (high << 19);
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004100ff, FUN_004100ff, "regs(ebx:ptr[0x40]:bytes, ecx:u32[0..15], esi:ptr[0x320]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
