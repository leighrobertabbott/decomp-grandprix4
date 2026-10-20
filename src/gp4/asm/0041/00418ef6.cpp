#include <gp4/gp4.h>

// Rebuilds the status bit set at +4 of the edi record from flag bits of the esi record
// (bit 0x08 <- esi+0x162 &8, 0x02 <- esi+0x7b &2, 0x04 <- esi+0xce &1, 0x10 <- esi+0x9d &0x20,
//  0x20 <- esi+0x9c &4, 0x40 <- esi+0x7b &0x10, 0x01 <- esi+0x7a &0x80). Bits 0..6 are recomputed
// from scratch; the higher bits of the dword are preserved. Flags on exit come from the last
// flag writer: the TEST of esi+0x7a (bit clear) or the final OR of the dword at edi+4.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// FUNCTION: GP4 0x00418ef6
void __cdecl FUN_00418ef6(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t status = GP4_FIELD(uint32_t, dst, 0x04);

    status &= 0xfffffff7u;
    if (GP4_FIELD(uint8_t, src, 0x162) & 0x08u) status |= 0x08u;
    status &= 0xfffffffdu;
    if (GP4_FIELD(uint8_t, src, 0x7b) & 0x02u) status |= 0x02u;
    status &= 0xfffffffbu;
    if (GP4_FIELD(uint8_t, src, 0xce) & 0x01u) status |= 0x04u;
    status &= 0xffffffefu;
    if (GP4_FIELD(uint8_t, src, 0x9d) & 0x20u) status |= 0x10u;
    status &= 0xffffffdfu;
    if (GP4_FIELD(uint8_t, src, 0x9c) & 0x04u) status |= 0x20u;
    status &= 0xffffffbfu;
    if (GP4_FIELD(uint8_t, src, 0x7b) & 0x10u) status |= 0x40u;
    status &= 0xfffffffeu;

    uint32_t flags;
    if (GP4_FIELD(uint8_t, src, 0x7a) & 0x80u) {
        status |= 0x01u;
        // OR result flags: CF = OF = AF = 0, ZF = 0 (bit 0 set), SF = bit 31, PF of low byte.
        flags = parity_flag(status) | ((status & 0x80000000u) ? 0x80u : 0u);
    } else {
        // TEST result 0: ZF = 1, PF = 1, SF = 0, CF = OF = AF = 0.
        flags = 0x44u;
    }
    GP4_FIELD(uint32_t, dst, 0x04) = status;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418ef6, FUN_00418ef6, "regs(edi:ptr[0x40]:bytes, esi:ptr[0x16a]:bytes) -> (cf, pf, af, zf, sf, of)")
