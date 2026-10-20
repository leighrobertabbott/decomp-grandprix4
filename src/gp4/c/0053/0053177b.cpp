#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of device record `index` of the 0x3c-byte records at 0x00827160.
static __declspec(noinline) uint32_t RecordAddress(uint32_t index) {
    return 0x00827160u + index * 0x3cu;
}

// Twin of 0x00531751 for bit 3: reports through the carry flag whether the record after the current
// device [0x007ad86c] has bit 3 set in its flag dword (+8): CF = 1 (stc) when it does or when the
// renderer byte [0x007ad869] is zero, CF = 0 (clc) otherwise. The other flags are those of
// `cmp byte [0x7ad869], 0` or of the `test dword [record + 8], 8`. eax is saved and restored.
// FUNCTION: GP4 0x0053177b
void __cdecl FUN_0053177b(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007ad869);
    uint32_t arith = gp4::flags::sub8(enabled, 0u);
    uint32_t carry = 1u;
    if (enabled != 0u) {
        const uint32_t record = RecordAddress(GP4_GLOBAL(uint32_t, 0x007ad86c) + 1u);
        const uint32_t bits = GP4_FIELD(uint32_t, record, 8) & 8u;
        arith = gp4::flags::logic32(bits);
        carry = (bits != 0u) ? 1u : 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x0053177b, FUN_0053177b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648")
