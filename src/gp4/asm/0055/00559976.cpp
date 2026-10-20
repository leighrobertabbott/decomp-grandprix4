#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Assertion walk over the 0x160-byte ring records from edi up to (not including) ebp: every
// record's dword +0x44 must be nonzero (zero is an int3). edi ends equal to ebp, so the flags are
// those of the final `cmp edi, ebp` (equal: ZF = PF = 1).
// FUNCTION: GP4 0x00559976
void __cdecl FUN_00559976(gp4::Regs* r) {
    uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;

    do {
        if (GP4_FIELD(uint32_t, edi, 0x44) == 0u) {
            __debugbreak();
        }
        edi += 0x160u;
    } while (edi != ebp);

    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(edi, ebp));
}
GP4_IMPL(0x00559976, FUN_00559976, "regs(edi:rec[0x8943a0,0x160,0..11], ebp:rel[edi,0x160,1..12]) -> (edi, cf, pf, af, zf, sf, of); globals=layout=state_00412c29")
