#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores ebx at the (unaligned) dword 0x00833cc5, then compares eax with the signed limit at
// 0x00822248: at or above it the routine 0x0054e232 is called inside a pushfd/pushal save (the
// original loads esi with the address of 0x0054cb90 first; 0x0054e232 is a bare `ret`, so esi is
// never read); below it eax is stored at 0x00833cc9 and the byte 0x00833f74 is set to 0xff.
// Every register is preserved; the flags are those of the final `cmp eax, [0x822248]`.
// FUNCTION: GP4 0x0054cb7e
void __cdecl FUN_0054cb7e(gp4::Regs* r) {
    const int32_t value = (int32_t)r->eax;

    GP4_GLOBAL(uint32_t, 0x00833cc5) = r->ebx;

    const int32_t limit = (int32_t)GP4_GLOBAL(uint32_t, 0x00822248);
    if (value >= limit) {
        gp4::Regs c = {};
        c.eflags = r->eflags;
        gp4::call_regs(0x0054e232, c);
    }

    const int32_t limit2 = (int32_t)GP4_GLOBAL(uint32_t, 0x00822248);
    if (value < limit2) {
        GP4_GLOBAL(uint32_t, 0x00833cc9) = (uint32_t)value;
        GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32((uint32_t)value, (uint32_t)limit2));
}
GP4_IMPL(0x0054cb7e, FUN_0054cb7e, "regs(eax:u32, ebx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
