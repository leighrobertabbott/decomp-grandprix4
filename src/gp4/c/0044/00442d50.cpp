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

// pushal ... popal around the release of the current car record [0x0066d820]: every register is
// restored, only the flags of the last flag writer survive.
// FUNCTION: GP4 0x00442d50
void __cdecl FUN_00442d50(gp4::Regs* r) {
    const uint32_t current = GP4_GLOBAL(uint32_t, 0x0066d820);
    uint32_t flags = logicFlags32(current);
    if (current != 0u) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(current));
        GP4_FIELD(uint8_t, record, 0xe5) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xe5) | 4u);

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.esi = current;
        gp4::call_regs(0x00410d92, c);

        GP4_FIELD(uint8_t, record, 0x120) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x120) + 1u);

        c.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
        gp4::call_regs(0x0043cdf8, c);

        c.edx = (c.edx & 0xffffff00u) | GP4_FIELD(uint8_t, record, 0x7c);
        gp4::call_regs(0x0043ce04, c);
        GP4_FIELD(uint16_t, record, 0xf6) = static_cast<uint16_t>(c.eax);

        c.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
        gp4::call_regs(0x0043cdfe, c);
        GP4_FIELD(uint16_t, record, 0x10e) = static_cast<uint16_t>(c.eax);

        gp4::call_regs(0x00442cd4, c);

        if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
            gp4::call_regs(0x00444d00, c);
        }
        gp4::call_regs(0x0040f9e6, c);

        GP4_GLOBAL(uint32_t, 0x0066d820) = 0u;
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x8d) & 0xdfu);
        GP4_FIELD(uint8_t, record, 0x8d) = value;
        flags = logicFlags8(value);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00442d50, FUN_00442d50, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+words_0062d210+state_0066d8c0+cars_00443648+state_0040fa01")
