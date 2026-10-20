#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mask update on a record at edi with the bit mask in dl: the bits are cleared from the byte +0x2c, then
// the byte +0x30 and +0x3c get the bits ORed in when the same bits are set in the byte at edi - 0x134,
// or cleared when they are not. dh is left as ~dl; the flags are those of the last and/or on +0x3c.
// FUNCTION: GP4 0x00559391
void __cdecl FUN_00559391(gp4::Regs* r) {
    const uint8_t mask = static_cast<uint8_t>(r->edx);
    const uint8_t inverse = static_cast<uint8_t>(~mask);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_FIELD(uint8_t, record, 0x2c) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x2c) & inverse);

    uint8_t last;
    if ((GP4_FIELD(uint8_t, record, -0x134) & mask) != 0u) {
        GP4_FIELD(uint8_t, record, 0x30) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x30) | mask);
        last = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x3c) | mask);
    } else {
        GP4_FIELD(uint8_t, record, 0x30) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x30) & inverse);
        last = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x3c) & inverse);
    }
    GP4_FIELD(uint8_t, record, 0x3c) = last;

    r->edx = (r->edx & 0xffff00ffu) | (static_cast<uint32_t>(inverse) << 8);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(last));
}
GP4_IMPL(0x00559391, FUN_00559391, "regs(edx:u32, esi:ptr[0x180]:bytes, edi:rel[esi,1,308..316]) -> (edx, cf, pf, af, zf, sf, of)")
