#include <gp4/gp4.h>
#include <gp4/flags.h>

// (a * b) >> 14 on the signed 64-bit product, low 32 bits kept (imul / shrd eax, edx, 14); the high
// half of the product is reported through *high (the original leaves it in edx).
static inline uint32_t term(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Rescales six Q14 screen-layout dwords from the 640x480 design resolution to the current one: the x
// scale (the dword [0x7a8ea2] << 14) / 640 (kept in [0x875270] and ecx) multiplies 0x874f0c / 0x874f20 /
// 0x874f28, the y scale ([0x7a8ea6] << 14) / 480 (ebp) multiplies 0x874f10 / 0x874f30 / 0x874f2c; each
// product is shifted right by 14, biased by 2 and rounded down to a multiple of 4. The dividends are
// zero-extended (xor edx, edx). eax is the last result, edx the high half of the last product, ecx / ebp
// the two scales; the flags are those of the last `and eax, 0xfffffffc`.
// FUNCTION: GP4 0x00525eba
void __cdecl FUN_00525eba(gp4::Regs* r) {
    int32_t remainder;
    const int32_t scaleX = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x007a8ea2) << 14), 0x280, &remainder);
    const int32_t scaleY = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x007a8ea6) << 14), 0x1e0, &remainder);
    GP4_GLOBAL(int32_t, 0x00875270) = scaleX;

    uint32_t high;
    uint32_t value = (term(GP4_GLOBAL(int32_t, 0x00874f0c), scaleX, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f0c) = value;
    value = (term(GP4_GLOBAL(int32_t, 0x00874f10), scaleY, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f10) = value;
    value = (term(GP4_GLOBAL(int32_t, 0x00874f20), scaleX, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f20) = value;
    value = (term(GP4_GLOBAL(int32_t, 0x00874f30), scaleY, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f30) = value;
    value = (term(GP4_GLOBAL(int32_t, 0x00874f28), scaleX, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f28) = value;
    value = (term(GP4_GLOBAL(int32_t, 0x00874f2c), scaleY, &high) + 2u) & 0xfffffffcu;
    GP4_GLOBAL(uint32_t, 0x00874f2c) = value;

    r->eax = value;
    r->ecx = static_cast<uint32_t>(scaleX);
    r->edx = high;
    r->ebp = static_cast<uint32_t>(scaleY);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00525eba, FUN_00525eba, "regs() -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
