#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the data of texture-table entry (0x20 bytes at 0x006975ec + word 0x009e2188) four times:
// entry +0xc is the source pointer, entry +0x10 the word length; the copies go to 0x0096935c,
// 0x0096945c, ... with both pointers advancing by 0x100. A word of 0xffff means "no entry".
// pushal/popal wrapper: every GPR is restored; the flags are those of the last flag writer
// (popal does not restore them): "cmp bx,-1" when nothing is copied, else the final "dec ebp".
// FUNCTION: GP4 0x0051ccd6
void __cdecl FUN_0051ccd6(gp4::Regs* r) {
    const uint32_t offset = GP4_ARRAY(uint16_t, 0x009e1b20)[0x334];
    uint32_t arith = gp4::flags::sub16(offset, 0xffffu);
    if (offset != 0xffffu) {
        const uint8_t* entry = GP4_ARRAY(uint8_t, 0x006975ec) + offset;
        const uint8_t* src = (const uint8_t*)(uintptr_t) * (const uint32_t*)(entry + 0xc);
        const uint32_t length = *(const uint16_t*)(entry + 0x10);
        uint8_t* dst = GP4_ARRAY(uint8_t, 0x0096935c);
        uint32_t remaining = 4u;
        do {
            for (uint32_t i = 0u; i < length; ++i) {
                dst[i] = src[i];
            }
            dst += 0x100;
            arith = gp4::flags::add32((uint32_t)(uintptr_t)src, 0x100u);
            src += 0x100;
            arith = gp4::flags::dec32(remaining, arith);
            --remaining;
        } while (remaining != 0u);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051ccd6, FUN_0051ccd6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
