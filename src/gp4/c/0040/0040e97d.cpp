#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags left by a TEST/OR: CF = OF = AF = 0, PF from the low byte, ZF/SF at the operand width.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Arithmetic flags of the 16-bit `left - right` (cmp).
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag8(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// When byte 0x7c2ea9 has bit 0x40 set, dword 0x66d8cc is zero and the signed word 0x7c2eb8 is
// below 2, flags the record in esi: byte +0x8d |= 4 and byte +0xcc |= 0x10. The flags are those
// of the last flag writer on the path taken.
// FUNCTION: GP4 0x0040e97d
void __cdecl FUN_0040e97d(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u;
    flags = logicFlags8(mode);                                           // test byte, 0x40
    if (mode != 0u) {
        const uint32_t lock = GP4_GLOBAL(uint32_t, 0x0066d8cc);
        flags = logicFlags32(lock);                                      // test dword, -1
        if (lock == 0u) {
            const uint32_t word = GP4_GLOBAL(uint16_t, 0x007c2eb8);
            flags = subFlags16(word, 2u);                                // cmp word, 2
            if (static_cast<int16_t>(word) < 2) {
                GP4_FIELD(uint8_t, record, 0x8d) |= 4u;
                const uint8_t status = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xcc) | 0x10u);
                GP4_FIELD(uint8_t, record, 0xcc) = status;
                flags = logicFlags8(status);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e97d, FUN_0040e97d, "regs(esi:ptr[0xd4]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
