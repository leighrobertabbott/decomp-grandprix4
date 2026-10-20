#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reciprocal of a cosine: takes the signed 16-bit angle word 0x008911f8, clamps it to
// [0x800, 0x35b4], looks up the Q14 cosine word at byte offset (angle >> 2) & 0xfffe of the table
// 0x007c5c5c and stores 0x10000000 / cos in 0x00811924. eax = quotient, edx = remainder,
// ebp = sign-extended cosine; the flags are those of `and eax, 0xfffe` (idiv leaves them).
// FUNCTION: GP4 0x00559186
void __cdecl FUN_00559186(gp4::Regs* r) {
    int16_t angle = GP4_GLOBAL(int16_t, 0x008911f8);
    if (angle > 0x35b4) {
        angle = 0x35b4;
    }
    if (angle < 0x800) {
        angle = 0x800;
    }
    if (angle < 0) {
        angle = (int16_t)-angle;
    }
    const uint32_t offset = ((uint32_t)(uint16_t)angle >> 2) & 0xfffeu;
    const int32_t cosine = *(const int16_t*)(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(0x10000000, cosine, &remainder);
    GP4_GLOBAL(int32_t, 0x00811924) = quotient;

    r->eax = (uint32_t)quotient;
    r->edx = (uint32_t)remainder;
    r->ebp = (uint32_t)cosine;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(offset));
}
GP4_IMPL(0x00559186, FUN_00559186, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
