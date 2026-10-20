#include <gp4/gp4.h>
#include <gp4/flags.h>

// Arm of mode byte == 0x80: the car-code copy and table reset (0x00522bad), the 0x882-byte clear
// (0x00522c66), the 22-byte copy (0x00522c96) and the flagged-car count (0x00522ab8).
static __declspec(noinline) void FullReset(gp4::Regs& c) {
    gp4::call_regs(0x00522bad, c);
    gp4::call_regs(0x00522c66, c);
    gp4::call_regs(0x00522c96, c);
    gp4::call_regs(0x00522ab8, c);
}

// Other arm: the car-code copy and table reset, then the flagged-car count.
static __declspec(noinline) void PartialReset(gp4::Regs& c) {
    gp4::call_regs(0x00522bad, c);
    gp4::call_regs(0x00522ab8, c);
}

// Resets the per-car tables: when the mode byte [0x007c2ea9] is 0x80 the full chain runs (including the
// 0x882-byte clear and the 22-byte copy), otherwise only the copy/table reset and the count. Registers and
// flags come from the last callee, 0x00522ab8 (eax, ebp = the two counts).
// FUNCTION: GP4 0x00522cb0
void __cdecl FUN_00522cb0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x80u));
    if (mode == 0x80u) {
        FullReset(c);
    } else {
        PartialReset(c);
    }
    *r = c;
}
GP4_IMPL(0x00522cb0, FUN_00522cb0, "regs() -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0066d8f4+list_00443278+bits_005174cb")
