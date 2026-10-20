#include <gp4/gp4.h>

// FUNCTION: GP4 0x00406ac7
// Walks the 0x160-byte record ring from edi. 0x4067ea reports in CF whether the record at edi
// matches. On a match (CF=1) it advances forward with 0x4069d2 while records keep matching and
// settles on the last matching record; otherwise it steps backwards with 0x4069e6 until a
// record matches. The result is left in 0x00629f04 (eax) and 0x00629f0c (edi).
void __cdecl FUN_00406ac7(gp4::Regs* r) {
    gp4::Regs s = *r;
    gp4::call_regs(0x004067ea, s);
    if ((s.eflags & gp4::CF) != 0u) {
        do {
            GP4_GLOBAL(uint32_t, 0x006278d4) = s.edi;
            GP4_GLOBAL(uint32_t, 0x006278d8) = s.eax;
            gp4::call_regs(0x004069d2, s);
            gp4::call_regs(0x004067ea, s);
        } while ((s.eflags & gp4::CF) != 0u);
        s.edi = GP4_GLOBAL(uint32_t, 0x006278d4);
        s.eax = GP4_GLOBAL(uint32_t, 0x006278d8);
    } else {
        do {
            gp4::call_regs(0x004069e6, s);
            gp4::call_regs(0x004067ea, s);
        } while ((s.eflags & gp4::CF) == 0u);
    }
    GP4_GLOBAL(uint32_t, 0x00629f04) = s.eax;
    GP4_GLOBAL(uint32_t, 0x00629f0c) = s.edi;
    *r = s;
}
GP4_IMPL(0x00406ac7, FUN_00406ac7, "regs(edi:u32[8995744..8995744]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00406ac7")
