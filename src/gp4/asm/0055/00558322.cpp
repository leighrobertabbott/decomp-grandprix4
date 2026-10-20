#include <gp4/gp4.h>

// Copies six fields of the 0x80-byte record at edi into a block at 0x008758b0: dwords +0x3c,
// +0x30, +0x34, +0x38, +0x40 go to 0x8758b0/b4/b8/bc/c0, the word +0x78 to 0x8758c4, and the byte
// at 0x8758c6 is set to 0xff. eax is saved and restored; no flags are touched.

// FUNCTION: GP4 0x00558322
void __cdecl FUN_00558322(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint16_t, 0x008758c4) = *reinterpret_cast<const uint16_t*>(record + 0x78);
    GP4_GLOBAL(uint32_t, 0x008758b0) = *reinterpret_cast<const uint32_t*>(record + 0x3c);
    GP4_GLOBAL(uint32_t, 0x008758b4) = *reinterpret_cast<const uint32_t*>(record + 0x30);
    GP4_GLOBAL(uint32_t, 0x008758b8) = *reinterpret_cast<const uint32_t*>(record + 0x34);
    GP4_GLOBAL(uint32_t, 0x008758bc) = *reinterpret_cast<const uint32_t*>(record + 0x38);
    GP4_GLOBAL(uint32_t, 0x008758c0) = *reinterpret_cast<const uint32_t*>(record + 0x40);
    GP4_GLOBAL(uint8_t, 0x008758c6) = 0xffu;
}
GP4_IMPL(0x00558322, FUN_00558322, "regs(edi:ptr[0x80]:bytes) -> (); globals=fuzz:i32")
