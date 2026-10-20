#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks bit 7 of byte +0x64 in a run of 0x160-byte segment records that ends
// at the record [0x866ef0] and extends backwards: the run length is the word
// at 0x62d22a (via 0x40b0a4) clamped to 1..0x100. Leaves eax = the unclamped
// count, ecx = 0, edi = first record below the run, and the flags of the final
// `sub edi, 0x160`.

// FUNCTION: GP4 0x005547e6
void __cdecl FUN_005547e6(gp4::Regs* r) {
    const int32_t raw = (int32_t)gp4::cdecl_<0x0040b0a4, uint32_t>();
    int32_t count = raw;
    if (count < 1) count = 1;
    if (count > 0x100) count = 0x100;
    uint32_t rec = GP4_GLOBAL(uint32_t, 0x00866ef0);
    uint32_t flags = 0;
    do {
        GP4_FIELD(uint8_t, rec, 0x64) |= 0x80;
        flags = gp4::flags::sub32(rec, 0x160u);
        rec -= 0x160u;
        --count;
    } while (count != 0);
    r->eax = (uint32_t)raw;
    r->ecx = 0;
    r->edi = rec;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005547e6, FUN_005547e6, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=win_0042b10d+word_0062d22a")
