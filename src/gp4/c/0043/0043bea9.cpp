#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043bea9
// Fills every unset (0xff) entry of the 22-byte table 0x0066d94c: for driver i it calls 0x0043bbad
// with eax = 2 (which temporarily sets byte +0 of the driver's record to 2 and returns the record
// pointer left in 0x00601f24); the entry becomes the signed byte at pointer + 8 when that is below 2,
// else the default byte 0x0062d27c. pushal/popal restore every register; popal does not restore the
// flags, which are those of the loop's final `cmp esi, 0x16` (zero: ZF and PF).
void __cdecl FUN_0043bea9(gp4::Regs* r) {
    for (uint32_t driver = 0u; driver != 0x16u; ++driver) {
        if (GP4_ARRAY(uint8_t, 0x0066d94c)[driver] == 0xffu) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.esi = driver;
            call.eax = 2u;
            gp4::call_regs(0x0043bbad, call);

            const int8_t* record = reinterpret_cast<const int8_t*>(static_cast<uintptr_t>(call.eax));
            int8_t value = record[8];
            if (value >= 2) {
                value = GP4_GLOBAL(int8_t, 0x0062d27c);
            }
            GP4_ARRAY(uint8_t, 0x0066d94c)[driver] = static_cast<uint8_t>(value);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bea9, FUN_0043bea9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+codes_0043ba12+tbl_0066d94c")
