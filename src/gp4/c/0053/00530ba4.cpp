#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks at the first 0x3c-byte record of the table at 0x827160 (the scan counter starts at 1, so only
// one record is visited): eax = 0xffffffff when its byte +5 is 1, else 0. ebx, ecx and edx are saved
// and restored; the flags are those of the loop test `or eax, eax` (or `or ecx, ecx` with ecx = 0
// when eax is still zero, the same flags).
// FUNCTION: GP4 0x00530ba4
void __cdecl FUN_00530ba4(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x00827160);
    const uint32_t result = GP4_FIELD(uint8_t, record, 5) == 1u ? 0xffffffffu : 0u;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result));
}
GP4_IMPL(0x00530ba4, FUN_00530ba4, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=rec_00530ae7")
