#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: 0x0040824b gives |dword +0x98| >> 8 of the selected car [0x00679d30]; that is
// shifted right (sar) by 8, multiplied by 0x2ba and shifted right (shr) by 16 into [0x00a6987c].
// popal restores every register; the flags are those of the final `shr eax, 16`.
// FUNCTION: GP4 0x00516667
void __cdecl FUN_00516667(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0040824b, c);

    const int32_t shifted = static_cast<int32_t>(c.eax) >> 8;
    const uint32_t product = static_cast<uint32_t>(shifted) * 0x2bau;
    const uint32_t result = product >> 16;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = result;

    const uint32_t previous = product >> 15;
    uint32_t flags = gp4::flags::logic32(result);
    flags |= previous & 1u ? gp4::flags::CF : 0u;
    flags |= ((previous ^ result) >> 31) & 1u ? gp4::flags::OF : 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516667, FUN_00516667, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")
