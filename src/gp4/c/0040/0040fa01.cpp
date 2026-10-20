#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags left by an OR / AND / TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// 0x0040f9e6 on the record, then clear bit 7 of its byte +0x78; returns the flags of the AND.
static inline uint32_t releaseRecord(const gp4::Regs* r, uint8_t* record) {
    gp4::Regs c = *r;
    c.esi = reinterpret_cast<uint32_t>(record);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0040f9e6, c);
    const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x78) & 0x7fu);
    GP4_FIELD(uint8_t, record, 0x78) = value;
    return logicFlags8(value);
}

// FUNCTION: GP4 0x0040fa01
// pushal ... popal: every register is restored, only the flags of the last flag writer survive.
void __cdecl FUN_0040fa01(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x0066d820));
    uint32_t flags = logicFlags32(reinterpret_cast<uint32_t>(record));
    if (record != 0) {
        flags = releaseRecord(r, record);
    }

    const uint8_t second = GP4_GLOBAL(uint8_t, 0x007aafd4);
    flags = logicFlags8(second);
    if (second != 0u) {
        record = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x0066d828));
        flags = logicFlags32(reinterpret_cast<uint32_t>(record));
        if (record != 0) {
            flags = releaseRecord(r, record);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040fa01, FUN_0040fa01, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040fa01")
