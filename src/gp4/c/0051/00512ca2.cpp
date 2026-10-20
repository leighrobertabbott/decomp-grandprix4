#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original indexes three dword tables with the constant edx = 2 ([edx*4 + base]); the compiler
// would fold that into absolute addresses and hide the table bases.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// Picks a ring position: eax = the current position (third entry of the table at 0x00679b34); when it
// is above the upper limit (third entry of 0x00679b24) it is replaced by the lower bound (third
// entry of 0x00679b9c). edx is saved and restored; the flags are those of that compare.
// FUNCTION: GP4 0x00512ca2
void __cdecl FUN_00512ca2(gp4::Regs* r) {
    int32_t position = tableEntry(GP4_ARRAY(int32_t, 0x00679b34), 2);
    const int32_t upper = tableEntry(GP4_ARRAY(int32_t, 0x00679b24), 2);
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(position), static_cast<uint32_t>(upper));
    if (position > upper) {
        position = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), 2);
    }
    r->eax = static_cast<uint32_t>(position);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512ca2, FUN_00512ca2, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=walk_00512cc1")
