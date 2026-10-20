#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic operation (or/and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Clears bit 7 of the flag byte +0x73 of the block at ebx and sets it in the block at edi. When the
// signed byte +0x8c of the edi block is not negative it is reset to 0x40 and bit 1 of its +0x73 is
// copied from the byte DAT_00627b65. The flags are those of the last compare/or/and.
// FUNCTION: GP4 0x0043fac9
void __cdecl FUN_0043fac9(gp4::Regs* r) {
    uint8_t* previous = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_FIELD(uint8_t, previous, 0x73) = static_cast<uint8_t>(GP4_FIELD(uint8_t, previous, 0x73) & 0x7fu);
    GP4_FIELD(uint8_t, body, 0x73) = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x73) | 0x80u);

    const uint32_t mark = GP4_FIELD(uint8_t, body, 0x8c);
    uint32_t flags;
    if (static_cast<int8_t>(mark) < 0) {
        // cmp byte [edi+0x8c], 0: CF = AF = OF = 0, the rest from the byte itself
        flags = logicFlags8(mark);
    } else {
        GP4_FIELD(uint8_t, body, 0x8c) = 0x40;
        uint32_t updated;
        if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
            updated = GP4_FIELD(uint8_t, body, 0x73) | 2u;
        } else {
            updated = GP4_FIELD(uint8_t, body, 0x73) & 0xfdu;
        }
        GP4_FIELD(uint8_t, body, 0x73) = static_cast<uint8_t>(updated);
        flags = logicFlags8(updated);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043fac9, FUN_0043fac9, "regs(ebx:ptr[0x7b]:bytes, edi:ptr[0x94]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
