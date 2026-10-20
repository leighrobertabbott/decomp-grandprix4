#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the car-record index of the pointer edi into 0x009339dc: (edi - 0x008943a0) / 0x160, a
// signed truncating division. pushal/popal restore every register; the flags are those of the
// `sub eax, 0x008943a0` (cdq and idiv leave them).
// FUNCTION: GP4 0x005524bd
void __cdecl FUN_005524bd(gp4::Regs* r) {
    const uint32_t pointer = r->edi;
    const int32_t offset = static_cast<int32_t>(pointer - 0x008943a0u);

    GP4_GLOBAL(int32_t, 0x009339dc) = offset / 0x160;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(pointer, 0x008943a0u));
}
GP4_IMPL(0x005524bd, FUN_005524bd, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
