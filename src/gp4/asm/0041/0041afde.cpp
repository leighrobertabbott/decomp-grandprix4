#include <gp4/gp4.h>

struct Unknown_0041afde {
    uint8_t Unknown00[0x202];
    uint8_t Slot202;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Clears the 12 qwords (0x60 bytes) of the slot selected by the byte at +0x202 in the
// 0x60-byte record table starting at 0x006020b0. The last flag writer is
// "imul eax, eax, 0x60" on a byte-sized index, so it cannot overflow (CF = OF = 0).
// FUNCTION: GP4 0x0041afde
void __cdecl FUN_0041afde(gp4::Regs* r) {
    const Unknown_0041afde* body = reinterpret_cast<const Unknown_0041afde*>(r->esi);
    const uint32_t offset = (uint32_t)body->Slot202 * 0x60u;

    double* slot = reinterpret_cast<double*>(0x006020b0u + offset);
    slot[0] = 0.0;
    slot[1] = 0.0;
    slot[2] = 0.0;
    slot[3] = 0.0;
    slot[4] = 0.0;
    slot[5] = 0.0;
    slot[6] = 0.0;
    slot[7] = 0.0;
    slot[8] = 0.0;
    slot[9] = 0.0;
    slot[10] = 0.0;
    slot[11] = 0.0;

    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(offset) | (offset == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x0041afde, FUN_0041afde, "regs(esi:ptr[0x20a]:bytes) -> (cf, pf, af, zf, sf, of)")
