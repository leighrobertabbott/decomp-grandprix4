#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055998b
// For each 0x160-byte record from edi up to (not including) ebp, walks the record ring cursor
// (starting at [0x00866f00], advancing +0x160 and wrapping from [0x00866ecc] back to 0x008943a0)
// until it finds a record whose dword +0x44 is zero or above the record's address (then dword
// +0x48 of the edi record gets bit 0x10000) or equal to the record's address (nothing is set).
// The ring cursor is kept across records. Hand-written register code: eax = the final cursor,
// edx = the last +0x44 value read, edi = ebp on return; the flags are those of the final
// `cmp edi, ebp` (equal: ZF = PF = 1).
void __cdecl FUN_0055998b(gp4::Regs* r) {
    uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;
    uint32_t eax = GP4_GLOBAL(uint32_t, 0x00866f00);
    uint32_t edx;

    do {
        for (;;) {
            edx = GP4_FIELD(uint32_t, eax, 0x44);
            if (edx == 0u || edi < edx) {
                GP4_FIELD(uint32_t, edi, 0x48) |= 0x10000u;
                break;
            }
            if (edi == edx) break;
            eax += 0x160u;
            if (eax == GP4_GLOBAL(uint32_t, 0x00866ecc)) eax = 0x008943a0u;
        }
        edi += 0x160u;
    } while (edi != ebp);

    r->eax = eax;
    r->edx = edx;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(edi, ebp));
}
GP4_IMPL(0x0055998b, FUN_0055998b, "regs(edi:rec[0x8943a0,0x160,0..11], ebp:rel[edi,0x160,1..12]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=records_008943a0+state_00412c29")
