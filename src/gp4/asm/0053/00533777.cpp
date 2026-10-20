#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatcher on the mode in ebx: 1, 2 and 3 run 0x00528623 with eax = 0, 1 and 2 (the callee leaves its registers
// and flags); any other mode only leaves the flags of `cmp ebx, 3`.
static __declspec(noinline) void RunMode0(gp4::Regs* r) {
    r->eax = 0u;
    gp4::call_regs(0x00528623, *r);
}

static __declspec(noinline) void RunMode1(gp4::Regs* r) {
    r->eax = 1u;
    gp4::call_regs(0x00528623, *r);
}

static __declspec(noinline) void RunMode2(gp4::Regs* r) {
    r->eax = 2u;
    gp4::call_regs(0x00528623, *r);
}

// FUNCTION: GP4 0x00533777
void __cdecl FUN_00533777(gp4::Regs* r) {
    const uint32_t mode = r->ebx;
    if (mode == 1u) {
        RunMode0(r);
    } else if (mode == 2u) {
        RunMode1(r);
    } else if (mode == 3u) {
        RunMode2(r);
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(mode, 3u));
    }
}
GP4_IMPL(0x00533777, FUN_00533777, "regs(ebx:u32{0,1,2,3,3,0x80000000}) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3")
