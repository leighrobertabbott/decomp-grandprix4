#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: looks the id [0x00ae6ca4] up in the slot table (0x00541d8d: eax = slot index or -1); for a
// hit it stores the performance counter (kernel32 QueryPerformanceCounter through the thunk 0x005a48be) in the
// qword 0x0083002b + 8 * index. popal restores every register; the flags are those of `cmp eax, -1` on a miss,
// otherwise of the `add eax, 0x83002b` that forms the destination address.
// FUNCTION: GP4 0x00540554
void __cdecl FUN_00540554(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
    gp4::call_regs(0x00541d8d, c);

    uint32_t arith = gp4::flags::sub32(c.eax, 0xffffffffu);
    if (c.eax != 0xffffffffu) {
        const uint32_t offset = c.eax << 3;
        arith = gp4::flags::add32(offset, 0x0083002bu);
        gp4::stdcall_<0x005a48be, int>(reinterpret_cast<void*>(static_cast<uintptr_t>(offset + 0x0083002bu)));
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00540554, FUN_00540554, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a")
