#include <gp4/gp4.h>
#include <gp4/flags.h>

// Palette entry -> packed 16-bit colour. eax (low byte) indexes a table of 3-word entries at 0x007c4d5c;
// the three words' top bits are packed 7:6:5 ( [w0>>9 : w1>>8(6 bits) : w2>>9(5 bits)] ), masked to 16 bits,
// and the global at 0x006e2464 is placed in the upper half of eax. The final flag writer is "or eax,ebx"
// (CF = OF = AF = 0, SZP of the result). ebx/ecx/edx are saved and restored.
// FUNCTION: GP4 0x00510c88
void __cdecl FUN_00510c88(gp4::Regs* r) {
    const uint32_t index = r->eax & 0xffu;
    const uint16_t* entry = (const uint16_t*)(uintptr_t)(0x007c4d5cu + index * 6u);

    const uint32_t w0 = (uint32_t)entry[0] >> 9;
    const uint32_t w1 = (uint32_t)entry[1] >> 8;
    const uint32_t w2 = (uint32_t)entry[2] >> 9;

    // shld ebx,ecx,6 ; shld ebx,edx,5 (ecx = w1<<26, edx = w2<<27)
    uint32_t packed = (w0 << 6) | (w1 & 0x3fu);
    packed = (packed << 5) | (w2 & 0x1fu);
    packed &= 0xffffu;

    const uint32_t result = (GP4_GLOBAL(uint32_t, 0x006e2464) << 16) | packed;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result));
}
GP4_IMPL(0x00510c88, FUN_00510c88, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
