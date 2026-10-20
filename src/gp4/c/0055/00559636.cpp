#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line: MSVC would fold `0x922060 - 0x87064c + 0x8705e8` into one constant and hide the globals.
static __declspec(noinline) uint32_t destinationStart(uint32_t destinationEnd, uint32_t sourceEnd, uint32_t sourceStart) {
    return destinationEnd - sourceEnd + sourceStart;
}

// Copies the 0x64 bytes (50 words, the last at 0x87064a -> 0x92205e) 0x008705e8..0x0087064c to 0x00921ffc,
// word by word. The loop test is `cmp ebx, 0x87064c; jb`, so the word at 0x87064a is still copied; edi is
// saved and restored; ebx ends at the source end and the flags are those of the final cmp (equal).
// FUNCTION: GP4 0x00559636
void __cdecl FUN_00559636(gp4::Regs* r) {
    uint16_t* out = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(destinationStart(0x00922060u, 0x0087064cu, 0x008705e8u)));
    const uint16_t* in = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(0x008705e8u));
    do {
        *out = *in;
        ++out;
        ++in;
    } while (reinterpret_cast<uintptr_t>(in) < 0x0087064cu);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(in));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->ebx, 0x0087064cu));
}
GP4_IMPL(0x00559636, FUN_00559636, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=words_00559636")
