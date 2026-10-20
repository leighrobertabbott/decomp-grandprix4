#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = number of set bits among bits 0..6 of the byte [ebx+6] (`shr dl,1; adc eax,0` seven times).
// ecx/edx are saved and restored. The flags are those of the last `dec ecx` (1 -> 0) with CF kept
// from the last `adc` (never a carry): CF=0 AF=0 OF=0 SF=0, ZF=PF=1.
// FUNCTION: GP4 0x00531340
void __cdecl FUN_00531340(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t bits = record[6];
    uint32_t count = 0;
    for (int i = 0; i < 7; ++i) {
        count += bits & 1u;
        bits >>= 1;
    }
    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x00531340, FUN_00531340, "regs(ebx:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
