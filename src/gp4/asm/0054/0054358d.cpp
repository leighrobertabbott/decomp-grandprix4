#include <gp4/gp4.h>
#include <gp4/flags.h>

// Edx-keyed handler: for the key 2 it resets the two handle dwords 0x008312cc and 0x008312d0 to -1 and, inside
// a pushal/popal pair, calls the stdcall routine 0x004fe160 (-1, &dword) for each of them. popal restores
// every register; the flags are those the second callee leaves. Any other key returns at once with the
// flags of `cmp edx, 2`.
// FUNCTION: GP4 0x0054358d
void __cdecl FUN_0054358d(gp4::Regs* r) {
    const uint32_t key = r->edx;
    const uint32_t compare = gp4::flags::sub32(key, 2u);
    if (key != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, compare);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x008312cc) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x008312d0) = 0xffffffffu;

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, compare);
    const uint32_t first[2] = {0xffffffffu, 0x008312ccu};
    gp4::call_regs_stdcall(0x004fe160, c, first);
    const uint32_t second[2] = {0xffffffffu, 0x008312d0u};
    gp4::call_regs_stdcall(0x004fe160, c, second);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054358d, FUN_0054358d, "regs(edx:u32{2,2,0,1,3,0x80000000}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
