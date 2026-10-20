#include <gp4/gp4.h>
#include <gp4/flags.h>

// Q14 cosine word of a 16-bit angle already folded into 0..0x4000: table offset (a >> 2) & 0xfffe.
static inline int32_t Q14Trig(uint32_t angle) {
    const uint32_t offset = (angle >> 2) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset));
}

// `shrd eax, edx, 0xe` after an imul: bits 14..45 of the 64-bit product.
static inline uint32_t Q14(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x005269ff
// Projects a car onto its track segment: the segment record edi = [esi + 0x1c] and its successor
// (+0x160) give two interpolated words, both blended by the car's Q14 fraction word [esi + 0x66]:
// x = wordA[+0] + ((next[+0] - wordA[+0]) * f >> 14) - [0x852d92] (the angle, flipped by 0x8000
// when bit 0x10 of [0x66d7a4] is set) and y = wordA[+2] + ((next[+2] - wordA[+2]) * f >> 14)
// (negated in that case). The angle is folded to 0..0x4000 and 0x4000 - y is folded the same way;
// the result is (Q14(cos(x') * cos(y')) * (word [0x679c08] << 3) >> 14) + eax, returned in eax.
// ecx leaves holding y, ebp the sign-extended first word, edx the entry eax (push/pop), edi the
// record; the flags are those of the final add.
void __cdecl FUN_005269ff(gp4::Regs* r) {
    const uint32_t entryEax = r->eax;
    const uint32_t car = r->esi;
    const uint32_t record = GP4_FIELD(uint32_t, car, 0x1c);
    const int32_t fraction = GP4_FIELD(int16_t, car, 0x66);

    const int32_t y0 = GP4_FIELD(int16_t, record, 2);
    const uint16_t y1 = GP4_FIELD(uint16_t, record, 0x162);
    const int32_t yDelta = static_cast<int16_t>(static_cast<uint16_t>(y1 - static_cast<uint16_t>(y0)));
    uint32_t ecx = static_cast<uint32_t>(y0) +
                   Q14(static_cast<int64_t>(yDelta) * static_cast<int64_t>(fraction));

    const int32_t x0 = GP4_FIELD(int16_t, record, 0);
    const uint16_t x1 = GP4_FIELD(uint16_t, record, 0x160);
    const int32_t xDelta = static_cast<int16_t>(static_cast<uint16_t>(x1 - static_cast<uint16_t>(x0)));
    const uint32_t blended = static_cast<uint32_t>(x0) +
                             Q14(static_cast<int64_t>(xDelta) * static_cast<int64_t>(fraction));
    // sub ax, [0x852d92]: only the low word matters from here on (and eax, 0xfffe).
    uint16_t angle = static_cast<uint16_t>(blended - GP4_GLOBAL(uint16_t, 0x00852d92));

    if ((GP4_GLOBAL(uint8_t, 0x0066d7a4) & 0x10u) != 0u) {
        angle = static_cast<uint16_t>(angle - 0x8000u);
        ecx = 0u - ecx;
    }
    if ((angle & 0x8000u) != 0u) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    if (angle >= 0x4000u) {
        angle = static_cast<uint16_t>(0u - angle);
        angle = static_cast<uint16_t>(angle + 0x8000u);
    }
    if ((angle & 0x8000u) != 0u) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const int32_t cosX = Q14Trig(angle);

    // mov dx, cx; neg dx; add dx, 0x4000
    uint16_t pitch = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(ecx));
    if ((pitch & 0x8000u) != 0u) {
        pitch = static_cast<uint16_t>(0u - pitch);
    }
    const int32_t cosY = Q14Trig(pitch);

    const uint32_t unit = Q14(static_cast<int64_t>(cosX) * static_cast<int64_t>(cosY));
    const uint32_t scale = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00679c08))) << 3;
    const uint32_t scaled = Q14(static_cast<int64_t>(static_cast<int32_t>(unit)) *
                                static_cast<int64_t>(static_cast<int32_t>(scale)));

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, entryEax));
    r->eax = scaled + entryEax;
    r->ecx = ecx;
    r->edx = entryEax;
    r->ebp = static_cast<uint32_t>(x0);
    r->edi = record;
}
GP4_IMPL(0x005269ff, FUN_005269ff, "regs(eax:u32, esi:ptr[0x238]:layout=car_0043c677) -> (eax, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+trackgeo_008943a0")
