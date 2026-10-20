#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears flag bits of the object record ebx with the mask dh: `and` on the bytes +0x2c, +0x30 and
// +0x3c, then, only when the byte flag 0x00874b3e is zero, on +0x34 and +0x38. The flags are those
// of the last `and` (+0x38 when it ran), else of `test byte [0x874b3e], 0xff`.
// FUNCTION: GP4 0x00559253
void __cdecl FUN_00559253(gp4::Regs* r) {
    const uint8_t mask = static_cast<uint8_t>(r->edx >> 8);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    record[0x2c] = static_cast<uint8_t>(record[0x2c] & mask);
    record[0x30] = static_cast<uint8_t>(record[0x30] & mask);
    record[0x3c] = static_cast<uint8_t>(record[0x3c] & mask);

    const uint8_t flag = GP4_GLOBAL(uint8_t, 0x00874b3e);
    uint32_t arith = gp4::flags::logic8(flag);
    if (flag == 0u) {
        record[0x34] = static_cast<uint8_t>(record[0x34] & mask);
        record[0x38] = static_cast<uint8_t>(record[0x38] & mask);
        arith = gp4::flags::logic8(record[0x38]);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00559253, FUN_00559253, "regs(ebx:ptr[0x44]:bytes, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
