#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043bd63
// pushal wrapper: for each of the 22 cars (esi = 0..21) 0x0043bb15 selects that car's setup
// sub-record (pointer in [0x00601f24]); its byte +8 is the level copied to byte +0x32, unless the
// level is below 2 (signed) and the car already has a level in the table at 0x0066d94c (0xff =
// not set), in which case the table level is stored. Every register comes back (popal); the flags
// are those of the final `cmp esi, 0x16` (equal: ZF and PF set).
void __cdecl FUN_0043bd63(gp4::Regs* r) {
    for (uint32_t car = 0; car != 0x16u; ++car) {
        gp4::Regs select = *r;
        select.st_in = 0;
        select.st_out = 0;
        select.eax = car;
        gp4::call_regs(0x0043bb15, select);

        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00601f24)));
        uint8_t level = GP4_FIELD(uint8_t, entry, 8);
        if (static_cast<int8_t>(level) < 2) {
            const uint8_t preset = GP4_ARRAY(uint8_t, 0x0066d94c)[car];
            if (preset != 0xffu) {
                level = preset;
            }
        }
        GP4_FIELD(uint8_t, entry, 0x32) = level;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bd63, FUN_0043bd63, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+codes_0043ba12+tbl_0066d94c")
