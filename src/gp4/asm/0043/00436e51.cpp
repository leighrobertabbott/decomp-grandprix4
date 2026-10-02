#include <gp4/gp4.h>

// FUNCTION: GP4 0x00436e51
void __cdecl FUN_00436e51(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    record[0x7b] &= 0x7fu;
    record[0x7a] &= 0xaeu;
    record[0xcc] &= 0x7fu;
    record[0xe7] &= 0xfeu;
    record[0x71] &= 0xefu;
    record[0x25f] &= 0x3fu;
    const uint32_t result = record[0x25f];
    uint32_t parity = result;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((~parity & 1u) << 2) | (result == 0 ? 0x40u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00436e51, FUN_00436e51, "regs(esi:ptr[0x267]:bytes) -> (cf,pf,af,zf,sf,of)")
