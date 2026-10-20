#include <gp4/gp4.h>

// Copies the low bytes of the six words at ebx+0x12, +0x14, +0x16, +0x18, +0x1a and +0x1c to the bytes
// at 0x008910a9, 0x008910a7, 0x008910a5, 0x008910a3, 0x008910a1 and 0x00891099 (edi = 0x0089108d plus
// 0x1c, 0x1a, 0x18, 0x16, 0x14 and 0xc). Exit: edi = 0x0089108d, the low word of eax = the last word read.
// The flags are untouched.
// FUNCTION: GP4 0x0051d4b5
void __cdecl FUN_0051d4b5(gp4::Regs* r) {
    const uint32_t ebx = r->ebx;
    const uint32_t edi = 0x0089108du;

    uint32_t word = GP4_FIELD(uint16_t, ebx, 0x12);
    GP4_FIELD(uint8_t, edi, 0x1c) = static_cast<uint8_t>(word);
    word = GP4_FIELD(uint16_t, ebx, 0x14);
    GP4_FIELD(uint8_t, edi, 0x1a) = static_cast<uint8_t>(word);
    word = GP4_FIELD(uint16_t, ebx, 0x16);
    GP4_FIELD(uint8_t, edi, 0x18) = static_cast<uint8_t>(word);
    word = GP4_FIELD(uint16_t, ebx, 0x18);
    GP4_FIELD(uint8_t, edi, 0x16) = static_cast<uint8_t>(word);
    word = GP4_FIELD(uint16_t, ebx, 0x1a);
    GP4_FIELD(uint8_t, edi, 0x14) = static_cast<uint8_t>(word);
    word = GP4_FIELD(uint16_t, ebx, 0x1c);
    GP4_FIELD(uint8_t, edi, 0xc) = static_cast<uint8_t>(word);

    r->eax = (r->eax & 0xffff0000u) | word;
    r->edi = edi;
}
GP4_IMPL(0x0051d4b5, FUN_0051d4b5, "regs(eax:u32, ebx:ptr[0x40]:bytes) -> (eax, edi)")
