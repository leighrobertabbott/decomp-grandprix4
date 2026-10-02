#include <gp4/gp4.h>

// imul dword + shrd eax, edx, 14 on a table entry times a global scale.
// Returns the shrd result; high is the imul high half (left in edx), flags
// follow the pinned oracle model for the undefined OF/AF of a multi-bit shrd.
static inline uint32_t scaleShift14(int32_t entry, int32_t scale, uint32_t& high, uint32_t& flags) {
    const int64_t product = static_cast<int64_t>(entry) * static_cast<int64_t>(scale);
    const uint64_t wide = static_cast<uint64_t>(product);
    const uint32_t low = static_cast<uint32_t>(wide);
    high = static_cast<uint32_t>(wide >> 32);

    const uint32_t result = static_cast<uint32_t>(wide >> 14);
    const uint32_t previous = static_cast<uint32_t>(wide >> 13);

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;

    flags = ((low >> 13) & 1u)                       // CF: last bit shifted out
        | ((parity & 1u) == 0u ? 0x4u : 0u)          // PF
        | (result == 0u ? 0x40u : 0u)                // ZF
        | ((result >> 31) << 7)                      // SF
        | (((previous ^ result) >> 31) << 11);       // OF (oracle model), AF = 0
    return result;
}

// FUNCTION: GP4 0x004108c0
void __cdecl FUN_004108c0(gp4::Regs* r) {
    uint8_t* src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t high;
    uint32_t flags;

    const uint32_t index0 = GP4_FIELD(uint16_t, src, 0x2c) & 0xffu;
    const uint32_t first = scaleShift14(GP4_ARRAY(int32_t, 0x007ad8bc)[index0], GP4_GLOBAL(int32_t, 0x00601d70), high, flags);
    GP4_FIELD(uint32_t, dst, 0x40c) = first;

    const uint32_t index1 = GP4_FIELD(uint16_t, src, 0x2e) & 0xffu;
    const uint32_t second = scaleShift14(GP4_ARRAY(int32_t, 0x007ad8e8)[index1], GP4_GLOBAL(int32_t, 0x00601d70), high, flags);
    GP4_FIELD(uint32_t, dst, 0x410) = second;

    r->eax = second;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004108c0, FUN_004108c0, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0x418]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
