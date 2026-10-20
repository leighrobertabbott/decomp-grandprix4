#include <gp4/gp4.h>
#include <gp4/flags.h>

// Ring record edi (0x160-byte records): byte +0x2b is a remembered value, 0xff meaning "unset". An
// unset byte takes the remembered value from the global byte 0x00871c78; any other value is stored
// there as the new remembered one. The byte is written back to the record (and left in al), then
// the per-record bit-counter routine 0x00557f2e runs for the same record; its register results and
// flags are kept.
// FUNCTION: GP4 0x00556d1b
void __cdecl FUN_00556d1b(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint8_t value = GP4_FIELD(uint8_t, record, 0x2b);
    if (value == 0xffu) {
        value = GP4_GLOBAL(uint8_t, 0x00871c78);
    } else {
        GP4_GLOBAL(uint8_t, 0x00871c78) = value;
    }
    GP4_FIELD(uint8_t, record, 0x2b) = value;
    r->eax = (r->eax & 0xffffff00u) | value;
    gp4::call_regs(0x00557f2e, *r);
}
GP4_IMPL(0x00556d1b, FUN_00556d1b, "regs(eax:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+bits_0055616f+flags2c_0055774b+ctr_00557f2e+rec2b_00556d1b")
