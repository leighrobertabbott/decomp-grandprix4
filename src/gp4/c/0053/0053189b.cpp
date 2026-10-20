#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of device record `index` of the 0x3c-byte records at 0x00827160.
static __declspec(noinline) uint32_t RecordAddress(uint32_t index) {
    return 0x00827160u + index * 0x3cu;
}

// When [0x007ad869] is non-zero, looks at the flag dword (+8) of the record after the current device
// [0x007ad86c]: bit 2 runs 0x00531711 (byte [ebx + 0x26] = 0xff) and bit 3 runs 0x00531721
// (byte [ebx + 0x1d] = 0), both on the caller's ebx. eax is saved and restored; the flags are those of
// `cmp byte [0x7ad869], 0` when disabled, else of the last `test dword [record + 8], 8` (the callees
// leave them untouched).
// FUNCTION: GP4 0x0053189b
void __cdecl FUN_0053189b(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007ad869);
    uint32_t arith = gp4::flags::sub8(enabled, 0u);
    if (enabled != 0u) {
        const uint32_t record = RecordAddress(GP4_GLOBAL(uint32_t, 0x007ad86c) + 1u);
        if ((GP4_FIELD(uint32_t, record, 8) & 4u) != 0u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            gp4::call_regs(0x00531711, c);
        }
        arith = gp4::flags::logic32(GP4_FIELD(uint32_t, record, 8) & 8u);
        if ((GP4_FIELD(uint32_t, record, 8) & 8u) != 0u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            gp4::call_regs(0x00531721, c);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053189b, FUN_0053189b, "regs(ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648")
