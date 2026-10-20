#include <gp4/gp4.h>
#include <gp4/flags.h>

// Collects the status bits shared by a 0x160-byte ring record (edi) and its predecessor: the
// predecessor is edi - 0x160, wrapped by the ring length [0x00866ebc] when that address lies below
// the ring start [0x00866ec0]. eax = (dword +0x2c of the record | dword +0x2c of the predecessor)
// & 0x08007800. edx is pushed and popped; the flags are those of the final `and`.
// FUNCTION: GP4 0x0055774b
void __cdecl FUN_0055774b(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint32_t previous = record - 0x160u;
    if (previous < GP4_GLOBAL(uint32_t, 0x00866ec0)) {
        previous += GP4_GLOBAL(uint32_t, 0x00866ebc);
    }
    const uint32_t bits = (GP4_FIELD(uint32_t, record, 0x2c) | GP4_FIELD(uint32_t, previous, 0x2c)) & 0x08007800u;
    r->eax = bits;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bits));
}
GP4_IMPL(0x0055774b, FUN_0055774b, "regs(edi:rec[0x8943a0,0x160,0..29]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+bits_0055616f+flags2c_0055774b")
