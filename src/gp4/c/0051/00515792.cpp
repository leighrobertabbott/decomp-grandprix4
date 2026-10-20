#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t slotIndex() {
    return 0x10u;
}

// Mode-0xd input setup (0x0051d160 clears the 16 flag bytes at 0x00866014 and runs the dispatcher),
// then eax = entry 0x10 of the dword table at 0x00891bd0, the empty routine 0x00515b63, bit 2 of the
// record byte [esi+0x8a] cleared and 0xff stored in 0x00954169. The flags are those of the final `and`.
// FUNCTION: GP4 0x00515792
void __cdecl FUN_00515792(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0051d160, c);

    r->eax = GP4_ARRAY(uint32_t, 0x00891bd0)[slotIndex()];
    gp4::cdecl_<0x00515b63, void>();

    uint8_t* rec = (uint8_t*)(uintptr_t)r->esi;
    const uint8_t cleared = (uint8_t)(rec[0x8a] & 0xfbu);
    rec[0x8a] = cleared;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x00515792, FUN_00515792, "regs(esi:ptr[0x92]:bytes, eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8+tab_00515a56")
