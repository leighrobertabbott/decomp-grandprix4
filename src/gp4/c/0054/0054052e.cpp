#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: looks the session handle [0x00ae6ca4] up in the 22-slot table with 0x00541d8d (slot index, or
// -1). For a hit it reads the performance counter into the qword stamp 0x0083002b + 8 * index (thunk 0x005a48be,
// kernel32!QueryPerformanceCounter) and adds 0x18 to the stamp's upper dword. popal restores every register, so
// only the flags escape: those of the final `add dword [stamp + 4], 0x18` on a hit, or of `cmp eax, -1`.
// FUNCTION: GP4 0x0054052e
void __cdecl FUN_0054052e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
    gp4::call_regs(0x00541d8d, c);

    uint32_t arith = gp4::flags::sub32(c.eax, 0xffffffffu);
    if (c.eax != 0xffffffffu) {
        const uint32_t stamp = (c.eax << 3) + 0x0083002bu;
        gp4::stdcall_<0x005a48be, int>(reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(stamp)));
        const uint32_t before = GP4_FIELD(uint32_t, stamp, 4);
        GP4_FIELD(uint32_t, stamp, 4) = before + 0x18u;
        arith = gp4::flags::add32(before, 0x18u);
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054052e, FUN_0054052e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stamps_0053fd63+slots_00541d5a+kind_00541cdb+slotsel_00540884")
