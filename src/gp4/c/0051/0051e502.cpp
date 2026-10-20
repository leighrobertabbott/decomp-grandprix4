#include <gp4/gp4.h>
#include <gp4/flags.h>

// (a * b) >> 14 on the signed 64-bit product, low 32 bits kept (imul / shrd eax, edx, 14); the high
// half of the product is what imul leaves in edx.
static inline uint32_t term(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Q14 matrix-vector product: the vector (x = [0x934df0], y = [0x934df4], z = ecx) times three rows
// of the matrix at 0x875294.. gives [0x8754e8] = x*m0 + z*m1 + y*m2, [0x8754ec] = x*m3 + z*m4 + y*m5
// and [0x8754f0] = x*m6 + z*m7 + y*m8 (each product shifted on its own); then [0x8754f0] * [0x875240]
// >> 14 is added to [0x8754ec]. The routine leaves eax = that last term, edx = its product's high half,
// ebp = the first two terms of the last row and the flags of the final memory add.
// FUNCTION: GP4 0x0051e502
void __cdecl FUN_0051e502(gp4::Regs* r) {
    const int32_t x = GP4_GLOBAL(int32_t, 0x00934df0);
    const int32_t z = static_cast<int32_t>(r->ecx);
    uint32_t high;

    uint32_t partial = term(x, GP4_GLOBAL(int32_t, 0x00875294), &high);
    partial += term(z, GP4_GLOBAL(int32_t, 0x00875298), &high);
    uint32_t sum = term(GP4_GLOBAL(int32_t, 0x00934df4), GP4_GLOBAL(int32_t, 0x0087529c), &high) + partial;
    GP4_GLOBAL(uint32_t, 0x008754e8) = sum;

    partial = term(GP4_GLOBAL(int32_t, 0x00934df0), GP4_GLOBAL(int32_t, 0x008752a0), &high);
    partial += term(z, GP4_GLOBAL(int32_t, 0x008752a4), &high);
    sum = term(GP4_GLOBAL(int32_t, 0x00934df4), GP4_GLOBAL(int32_t, 0x008752a8), &high) + partial;
    GP4_GLOBAL(uint32_t, 0x008754ec) = sum;

    partial = term(GP4_GLOBAL(int32_t, 0x00934df0), GP4_GLOBAL(int32_t, 0x008752ac), &high);
    partial += term(z, GP4_GLOBAL(int32_t, 0x008752b0), &high);
    sum = term(GP4_GLOBAL(int32_t, 0x00934df4), GP4_GLOBAL(int32_t, 0x008752b4), &high) + partial;
    GP4_GLOBAL(uint32_t, 0x008754f0) = sum;

    const uint32_t scaled = term(static_cast<int32_t>(sum), GP4_GLOBAL(int32_t, 0x00875240), &high);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x008754ec);
    GP4_GLOBAL(uint32_t, 0x008754ec) = before + scaled;

    r->eax = scaled;
    r->edx = high;
    r->ebp = partial;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, scaled));
}
GP4_IMPL(0x0051e502, FUN_0051e502, "regs(ecx:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
