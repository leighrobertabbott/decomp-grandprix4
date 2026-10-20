#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches a 5-bit code from eax: the byte 0x007a97b1 becomes (byte 0x007aafd9 & 0x80) | (eax & 0x1f), and
// the dword 0x007a97ac becomes the byte at 0x0083327d + min(eax & 0x1f, 0x1a). Exit state: dl = the stored
// byte (upper edx kept), ebx = the table address, eax = the table byte. The flags are those of
// `add ebx, 0x0083327d`.
// FUNCTION: GP4 0x0054aca4
void __cdecl FUN_0054aca4(gp4::Regs* r) {
    const uint32_t code = r->eax & 0x1fu;
    const uint32_t packed = (GP4_GLOBAL(uint8_t, 0x007aafd9) & 0x80u) | code;
    GP4_GLOBAL(uint8_t, 0x007a97b1) = static_cast<uint8_t>(packed);

    const uint32_t index = (code > 0x1au) ? 0x1au : code;
    const uint32_t entry = GP4_ARRAY(const uint8_t, 0x0083327d)[index];
    GP4_GLOBAL(uint32_t, 0x007a97ac) = entry;

    r->edx = (r->edx & 0xffffff00u) | packed;
    r->ebx = 0x0083327du + index;
    r->eax = entry;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(index, 0x0083327du));
}
GP4_IMPL(0x0054aca4, FUN_0054aca4, "regs(eax:u32, edx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
