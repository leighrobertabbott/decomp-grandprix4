#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that saves game-state slot eax: copies the word [0x007ad892] to [0x0066d8ba], runs
// the pointer-to-offset converter 0x00513982, copies [0x006975c0] bytes (as dwords) of the state
// block at 0x0066d810 into the slot at eax * [0x006975c4] + [0x007a8e18], tags the slot's first dword
// with 3, appends the 0xa5e-dword table copy of 0x0040b511 right behind the state bytes and finally
// converts the pointers back with 0x005139ed. popal restores every GPR; the flags are those the
// last callee leaves (the equal `cmp esi, ebx` of 0x005139ed).
// FUNCTION: GP4 0x00520b1b
void __cdecl FUN_00520b1b(gp4::Regs* r) {
    const uint32_t slot_index = r->eax;
    GP4_GLOBAL(uint16_t, 0x0066d8ba) = GP4_GLOBAL(uint16_t, 0x007ad892);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00513982, c);

    uint32_t* slot = reinterpret_cast<uint32_t*>(
        static_cast<uintptr_t>(slot_index * GP4_GLOBAL(uint32_t, 0x006975c4) + GP4_GLOBAL(uint32_t, 0x007a8e18)));
    uint32_t* dst = slot;
    const uint32_t* src = GP4_ARRAY(const uint32_t, 0x0066d810);
    for (uint32_t left = GP4_GLOBAL(uint32_t, 0x006975c0) >> 2; left != 0u; --left) {
        *dst++ = *src++;
    }
    *slot = 3u;

    c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    gp4::call_regs(0x0040b511, c);

    c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005139ed, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00520b1b, FUN_00520b1b, "regs(eax:u32[0..3]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0")
