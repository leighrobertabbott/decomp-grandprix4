#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b0ac
void __cdecl FUN_0040b0ac(gp4::Regs* r) {
    // rep movsb: copies the block 0x630224..0x63e18c to [edi]; eax is the advanced destination.
    // The flags are those of `sub ecx, esi` (0x63e18c - 0x630224 = 0xdf68):
    // CF = AF = ZF = SF = OF = 0 and PF = 0 (0x68 has three bits set).
    uint8_t* destination = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint8_t* source = GP4_ARRAY(uint8_t, 0x00630224);
    const uint8_t* end = GP4_ARRAY(uint8_t, 0x0063e18c);
    while (source != end) {
        *destination++ = *source++;
    }
    r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(destination));
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x0040b0ac, FUN_0040b0ac, "regs(edi:ptr[0xdf70]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=copy_00630224")
