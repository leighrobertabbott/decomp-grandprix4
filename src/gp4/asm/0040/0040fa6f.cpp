#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags left by a byte-wide OR/AND/TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t LogicFlags8(uint32_t result) {
    uint32_t flags = ParityFlag(result);
    if ((result & 0xffu) == 0u) flags |= 0x40u;
    if ((result & 0x80u) != 0u) flags |= 0x80u;
    return flags;
}

// FUNCTION: GP4 0x0040fa6f
void __cdecl FUN_0040fa6f(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    // test byte [esi+0x7c], 0xff ; jns -> nothing else happens
    const uint8_t mode = GP4_FIELD(uint8_t, record, 0x7c);
    uint32_t flags = LogicFlags8(mode);
    if ((mode & 0x80u) != 0u) {
        uint16_t value;
        if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && (mode & 0x40u) != 0u) {
            value = GP4_GLOBAL(uint16_t, 0x0066d8b6);
        } else {
            value = GP4_GLOBAL(uint16_t, 0x0066d8b4);
        }
        uint8_t state = GP4_FIELD(uint8_t, record, 0x78);
        state &= 0x80;
        state |= static_cast<uint8_t>(value) & 0x7f;
        flags = LogicFlags8(state);
        if ((GP4_FIELD(uint8_t, record, 0xe6) & 8u) == 0u) {
            state |= 0x20;
            flags = LogicFlags8(state);
        } else {
            flags = LogicFlags8(GP4_FIELD(uint8_t, record, 0xe6) & 8u);
        }
        GP4_FIELD(uint8_t, record, 0x78) = state;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040fa6f, FUN_0040fa6f, "regs(esi:ptr[0xee]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
