#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// The opcode flag byte of 0x00445218 (al) with bit 0 added when bit 7 of byte +0x78 of the record in esi is set.
// eax keeps its upper 24 bits; the flags are those of the test (or of the `or al, 1`).
// FUNCTION: GP4 0x0044520a
void __cdecl FUN_0044520a(gp4::Regs* r) {
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00445218, call);

    uint32_t flag = call.eax & 0xffu;
    const uint32_t state = record[0x78] & 0x80u;
    uint32_t flags = logicFlags8(state);                 // test byte [esi + 0x78], 0x80
    if (state != 0u) {
        flag |= 1u;
        flags = logicFlags8(flag);                       // or al, 1
    }
    r->eax = (call.eax & 0xffffff00u) | flag;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044520a, FUN_0044520a, "regs(eax:u32, esi:ptr[0x80]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=flags_00445218")
