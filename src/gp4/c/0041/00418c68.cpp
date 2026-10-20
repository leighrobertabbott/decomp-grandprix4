#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00418c68
void __cdecl FUN_00418c68(gp4::Regs* r) {
    // pushal ... popal: every register comes back, only the flags of the last "or [edi], 0x40" remain
    gp4::Regs call = *r;
    gp4::call_regs(0x00418901, call);
    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.eax));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_FIELD(uint32_t, entry, 0x98) = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00629dce))) << 8;
    GP4_FIELD(uint32_t, entry, 0x9c) = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00629dd0))) << 8;

    uint32_t bits = GP4_FIELD(uint32_t, entry, 0x00) & 0xffffff7fu;
    if (GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
        bits |= 0x80u;
    }
    GP4_FIELD(uint32_t, entry, 0x00) = bits;

    GP4_FIELD(uint32_t, record, 0x254) = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint32_t, record, 0x254) = GP4_FIELD(uint32_t, record, 0x254) + 0x15eu;

    bits = (GP4_FIELD(uint32_t, entry, 0x00) & 0xffffffbfu) | 0x40u;
    GP4_FIELD(uint32_t, entry, 0x00) = bits;

    // the final "or" leaves CF = OF = AF = 0; ZF is clear because bit 6 is set
    const uint32_t flags = parityFlag(bits)
        | ((bits & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418c68, FUN_00418c68, "regs(esi:ptr[0x260]:layout=rec_00418c68) -> (cf, pf, af, zf, sf, of); globals=layout=state_00418c68")
