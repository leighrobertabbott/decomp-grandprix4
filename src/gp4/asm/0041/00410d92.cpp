#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410d92
void __cdecl FUN_00410d92(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    // The eight "and" instructions clear every bit of the byte at +0xe6.
    record[0xe6] = static_cast<uint8_t>(record[0xe6] & 0x7fu & 0xbfu & 0xdfu & 0xefu & 0xf7u & 0xfbu & 0xfdu & 0xfeu);

    // Last instruction: and byte [esi+0xcf], 0xdf leaves the flags (CF = OF = AF = 0).
    const uint8_t result = static_cast<uint8_t>(record[0xcf] & 0xdfu);
    record[0xcf] = result;

    uint32_t low = result;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    uint32_t flags = 0u;
    if ((low & 1u) == 0u) flags |= 0x4u;     // PF
    if (result == 0u) flags |= 0x40u;        // ZF
    if ((result & 0x80u) != 0u) flags |= 0x80u;  // SF
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410d92, FUN_00410d92, "regs(esi:ptr[0xee]:bytes) -> (cf, pf, af, zf, sf, of)")
