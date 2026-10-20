#include <gp4/gp4.h>

// Calls 0x00441d85 (marks or unmarks the 32 consecutive segment records from the lap start) with dh = byte [ebx+1]
// as its set / clear selector. pushal / popal restore every register; only the flags of the call remain.
// FUNCTION: GP4 0x00441d7a
void __cdecl FUN_00441d7a(gp4::Regs* r) {
    const uint8_t* const source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edx = (r->edx & 0xffff00ffu) | (static_cast<uint32_t>(source[1]) << 8);
    gp4::call_regs(0x00441d85, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441d7a, FUN_00441d7a, "regs(ebx:ptr[0x40]:u8[0..1], ecx:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+w1a_008943a0")
