#include <gp4/gp4.h>
#include <gp4/flags.h>

// Q14 cosine word table (16384 = 1.0), indexed by |angle| >> 3 (0x10000 angle units per turn).
static inline int32_t CosQ14(uint32_t angle) {
    uint16_t a = static_cast<uint16_t>(angle);
    if (static_cast<int16_t>(a) < 0) {
        a = static_cast<uint16_t>(0u - a);
    }
    const uint32_t off = (static_cast<uint32_t>(a >> 2)) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[off >> 1];
}

// (c * b) >> 14 clamped to a signed byte; *arith gets the flags of the final `cmp v, -0x80`.
static inline uint8_t ScaleClamp(int32_t c, int32_t b, uint32_t* arith) {
    int32_t v = (c * b) >> 14;
    if (v > 0x7f) {
        v = 0x7f;
    }
    *arith = gp4::flags::sub32(static_cast<uint32_t>(v), 0xffffff80u);
    if (v < -0x80) {
        v = -0x80;
    }
    return static_cast<uint8_t>(v);
}

// Ring-record rotation (pushal/popal wrapper): the angle words +0x6c/+0x6e of this 0x160-byte record
// and the previous one (edi-0xf4/-0xf2) are averaged; the signed bytes +0x84 and +0x87 are rotated by
// them into (+0x82, +0x84) and (+0x85, +0x87) with clamping to -0x80..0x7f. All registers are
// restored; the flags are those of the last clamp compare.
// FUNCTION: GP4 0x005552e9
void __cdecl FUN_005552e9(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t arith = 0;

    const int32_t b84 = GP4_FIELD(int8_t, rec, 0x84);
    const uint32_t angle1 = static_cast<uint32_t>(
        ((static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x6c)) +
          static_cast<int32_t>(GP4_FIELD(int16_t, rec, -0xf4))) >> 1) + 0x4000);
    const int32_t s1 = CosQ14(angle1);
    const int32_t c1 = CosQ14(0x4000u - angle1);
    GP4_FIELD(uint8_t, rec, 0x82) = ScaleClamp(c1, b84, &arith);
    GP4_FIELD(uint8_t, rec, 0x84) = ScaleClamp(s1, b84, &arith);

    const int32_t b87 = GP4_FIELD(int8_t, rec, 0x87);
    const uint32_t angle2 = static_cast<uint32_t>(
        ((static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x6e)) +
          static_cast<int32_t>(GP4_FIELD(int16_t, rec, -0xf2))) >> 1) - 0x4000);
    const int32_t s2 = CosQ14(angle2);
    const int32_t c2 = CosQ14(0x4000u - angle2);
    GP4_FIELD(uint8_t, rec, 0x85) = ScaleClamp(c2, b87, &arith);
    GP4_FIELD(uint8_t, rec, 0x87) = ScaleClamp(s2, b87, &arith);

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005552e9, FUN_005552e9, "regs(esi:ptr[0x2c0]:u16{0x8000,0x4000,0xc000,0x0080,0}, edi:rel[esi,0x160,1..1]) -> (cf, pf, af, zf, sf, of); cases=800")
