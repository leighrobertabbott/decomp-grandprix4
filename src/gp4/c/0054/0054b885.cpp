#include <gp4/gp4.h>
#include <gp4/flags.h>

// Initialises the eight 0x104-byte records at 0x007aa7b0: the dword +0 is set to 1 and the byte +4 to
// 0xe0 + index, then the byte 0x007aafd0 (the 9th record's first byte) is cleared. Leaves bl = 0xe8
// (upper bytes kept), ecx = 0 and edi = 0x007aafd0; the flags are those of the last `add edi, 0x104`.
// FUNCTION: GP4 0x0054b885
void __cdecl FUN_0054b885(gp4::Regs* r) {
    uint32_t edi = 0x007aa7b0u;
    uint32_t bl = 0xe0u;
    uint32_t arith = 0u;
    for (uint32_t n = 8u; n != 0u; --n) {
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(edi)) = 1u;
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi + 4u)) = static_cast<uint8_t>(bl);
        ++bl;
        arith = gp4::flags::add32(edi, 0x104u);
        edi += 0x104u;
    }
    GP4_GLOBAL(uint8_t, 0x007aafd0) = 0u;
    r->ebx = (r->ebx & 0xffffff00u) | (bl & 0xffu);
    r->ecx = 0u;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054b885, FUN_0054b885, "regs(ebx:u32) -> (ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
