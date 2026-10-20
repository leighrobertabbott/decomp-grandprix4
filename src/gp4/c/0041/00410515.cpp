#include <gp4/gp4.h>

// Flags of a logic op on a dword: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// (high nibble - 1 clamped to 0..10) << 4 | (low nibble - 1 clamped to 0..10)
static inline uint32_t packNibbles(uint32_t value) {
    int32_t high = static_cast<int32_t>(value >> 4) - 1;
    if (high < 0) {
        high = 0;
    }
    if (high > 10) {
        high = 10;
    }
    int32_t low = static_cast<int32_t>(value & 0xfu) - 1;
    if (low < 0) {
        low = 0;
    }
    if (low > 10) {
        low = 10;
    }
    return static_cast<uint32_t>(low) | (static_cast<uint32_t>(high) << 4);
}

// FUNCTION: GP4 0x00410515
// ebx = source record (words at +0x2c / +0x2e, high bytes used), esi = destination record
// (bytes +0x266 / +0x267 written). Leaves eax = the first packed byte and the flags of the
// final "or edx, ecx" (ecx and edx are restored).
void __cdecl FUN_00410515(gp4::Regs* r) {
    uint8_t* const source = reinterpret_cast<uint8_t*>(r->ebx);
    uint8_t* const dest = reinterpret_cast<uint8_t*>(r->esi);

    uint32_t first = 0;
    uint32_t second = 0;
    bool useDefaults = true;
    if (GP4_GLOBAL(uint8_t, 0x00601f1f) == 0xff) {
        first = static_cast<uint32_t>(GP4_FIELD(uint16_t, source, 0x2e)) >> 8;
        second = static_cast<uint32_t>(GP4_FIELD(uint16_t, source, 0x2c)) >> 8;
        useDefaults = (first & 0xffu) == 0;
    }
    if (useDefaults) {
        first = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0062f178)) + 0x11;
        second = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0062f179)) + 0x11;
    }

    const uint32_t firstPacked = packNibbles(first);
    GP4_FIELD(uint8_t, dest, 0x266) = static_cast<uint8_t>(firstPacked);
    const uint32_t secondPacked = packNibbles(second);
    GP4_FIELD(uint8_t, dest, 0x267) = static_cast<uint8_t>(secondPacked);

    r->eax = firstPacked;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(secondPacked);
}
GP4_IMPL(0x00410515, FUN_00410515, "regs(ebx:ptr[0x38]:bytes, esi:ptr[0x270]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
