#include <gp4/gp4.h>
#include <gp4/flags.h>

// Edits the flag bytes of the record at ebx: [+0x2c] |= dl; [+0x30] &= dh; [+0x3c] &= dh; and when
// the byte at 0x00874b3e is zero also [+0x34] &= dh and [+0x38] &= dh. The flags are those of the
// `test byte [0x874b3e], 0xff` or, when it is zero, of the last `and byte [ebx+0x38], dh`.
// FUNCTION: GP4 0x0055922b
void __cdecl FUN_0055922b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint8_t dl = static_cast<uint8_t>(r->edx);
    const uint8_t dh = static_cast<uint8_t>(r->edx >> 8);

    record[0x2c] = static_cast<uint8_t>(record[0x2c] | dl);
    record[0x30] = static_cast<uint8_t>(record[0x30] & dh);
    record[0x3c] = static_cast<uint8_t>(record[0x3c] & dh);

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00874b3e);
    uint32_t flags = gp4::flags::logic8(mode);
    if (mode == 0) {
        record[0x34] = static_cast<uint8_t>(record[0x34] & dh);
        record[0x38] = static_cast<uint8_t>(record[0x38] & dh);
        flags = gp4::flags::logic8(record[0x38]);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055922b, FUN_0055922b, "regs(ebx:ptr[0x44]:bytes, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
