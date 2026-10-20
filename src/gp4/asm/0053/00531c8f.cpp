#include <gp4/gp4.h>
#include <gp4/flags.h>

// Message handler keyed by edx. Code 2: loads the dword [0x00abc088] (the list selection index), passes it to
// the stdcall routine 0x004ff500 inside a pushal/popal pair (every register comes back, the flags are the
// callee's) and stores the saved value (popal restored eax) to [0x00827d6d]. Codes 5 and 6: clears
// [0x0082fd88] (the flags are those of the matching `cmp edx, 5` / `cmp edx, 6`). Any other code changes
// nothing but the flags of `cmp edx, 6`. Always returns with edx = 0.
// FUNCTION: GP4 0x00531c8f
void __cdecl FUN_00531c8f(gp4::Regs* r) {
    const uint32_t code = r->edx;
    uint32_t flags;
    if (code == 2u) {
        const uint32_t value = GP4_GLOBAL(uint32_t, 0x00abc088);
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(code, 2u));
        c.eax = value;
        const uint32_t args[1] = {value};
        gp4::call_regs_stdcall(0x004ff500, c, args);
        GP4_GLOBAL(uint32_t, 0x00827d6d) = value;
        r->eax = value;
        flags = c.eflags;
    } else {
        if (code == 5u) {
            flags = gp4::flags::sub32(code, 5u);
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0;
        } else {
            flags = gp4::flags::sub32(code, 6u);
            if (code == 6u) {
                GP4_GLOBAL(uint32_t, 0x0082fd88) = 0;
            }
        }
    }
    r->edx = 0;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00531c8f, FUN_00531c8f, "regs(edx:u32{2,5,6,0,1,3,4,7}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00543842")
