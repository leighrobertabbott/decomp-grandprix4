#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first free slot of the 16-entry table of 8-byte records at 0x00828020 (a record is free
// when its first dword is zero). Success: edi = the record, CF = 0 and the flags of `or ecx, ecx`
// on the number of slots left; none free: edi = 0, CF = 1 (stc over the flags of `or ecx, ecx` with
// ecx = 0). ecx is saved and restored.
// FUNCTION: GP4 0x005336c6
void __cdecl FUN_005336c6(gp4::Regs* r) {
    uint32_t edi = 0x00828020u;
    uint32_t left = 0x10u;
    while (left != 0u) {
        if (*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(edi)) == 0u) {
            break;
        }
        edi += 8u;
        --left;
    }

    uint32_t arith = gp4::flags::logic32(left);
    if (left != 0u) {
        arith &= ~static_cast<uint32_t>(gp4::flags::CF);
    } else {
        edi = 0u;
        arith |= gp4::flags::CF;
    }
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005336c6, FUN_005336c6, "regs() -> (edi, cf, pf, af, zf, sf, of); globals=layout=slots_005336c6")
