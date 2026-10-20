#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the marker byte [0x008758c6] is 0xff, restores the six fields saved at 0x008758b0..
// into the record at edi (word +0x78, dwords +0x3c, +0x30, +0x34, +0x38, +0x40; the inverse of
// the saver 0x00558322) and clears the marker. eax is saved and restored; the flags are those of
// `cmp byte [0x008758c6], 0xff`.
// FUNCTION: GP4 0x0055835e
void __cdecl FUN_0055835e(gp4::Regs* r) {
    const uint8_t marker = GP4_GLOBAL(uint8_t, 0x008758c6);
    if (marker == 0xffu) {
        const uint32_t record = r->edi;
        GP4_FIELD(uint16_t, record, 0x78) = GP4_GLOBAL(uint16_t, 0x008758c4);
        GP4_FIELD(uint32_t, record, 0x3c) = GP4_GLOBAL(uint32_t, 0x008758b0);
        GP4_FIELD(uint32_t, record, 0x30) = GP4_GLOBAL(uint32_t, 0x008758b4);
        GP4_FIELD(uint32_t, record, 0x34) = GP4_GLOBAL(uint32_t, 0x008758b8);
        GP4_FIELD(uint32_t, record, 0x38) = GP4_GLOBAL(uint32_t, 0x008758bc);
        GP4_FIELD(uint32_t, record, 0x40) = GP4_GLOBAL(uint32_t, 0x008758c0);
        GP4_GLOBAL(uint8_t, 0x008758c6) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(marker, 0xffu));
}
GP4_IMPL(0x0055835e, FUN_0055835e, "regs(edi:ptr[0x80]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
