#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043be59
// pushal wrapper: for each of the 22 cars (esi = 0..21) whose flag byte in the table at 0x007c2a48
// has bit 7 clear, 0x0043bb15 selects the car's setup sub-record (pointer in [0x00601f24]),
// 0x0043ca66 stores the car's interpolated value in [0x006227a0] and 0x0043ca8b gives the current
// level. A level below 2 (signed byte) is replaced by the default [0x0062d27c], or by the car's table
// level at 0x0066d94c when that is not 0xff. The level is stored at byte +0x32 of the sub-record.
// Every register comes back (popal); the flags are those of the final `cmp esi, 0x16` (ZF and PF).
void __cdecl FUN_0043be59(gp4::Regs* r) {
    for (uint32_t car = 0; car != 0x16u; ++car) {
        if (static_cast<int8_t>(GP4_ARRAY(uint8_t, 0x007c2a48)[car]) < 0) {
            continue;
        }

        gp4::Regs select = *r;
        select.st_in = 0;
        select.st_out = 0;
        select.eax = car;
        gp4::call_regs(0x0043bb15, select);
        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00601f24)));

        gp4::Regs position = *r;
        position.st_in = 0;
        position.st_out = 0;
        position.eax = car;
        gp4::call_regs(0x0043ca66, position);
        GP4_GLOBAL(uint32_t, 0x006227a0) = position.eax;

        gp4::Regs band = *r;
        band.st_in = 0;
        band.st_out = 0;
        gp4::call_regs(0x0043ca8b, band);

        uint8_t level = static_cast<uint8_t>(band.eax);
        if (static_cast<int8_t>(level) < 2) {
            level = GP4_GLOBAL(uint8_t, 0x0062d27c);
            const uint8_t preset = GP4_ARRAY(uint8_t, 0x0066d94c)[car];
            if (preset != 0xffu) {
                level = preset;
            }
        }
        GP4_FIELD(uint8_t, entry, 0x32) = level;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043be59, FUN_0043be59, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+codes_0043ba12+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043bcd4+state_0043bd97+state_0043be59")
