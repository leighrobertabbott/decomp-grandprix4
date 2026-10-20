#include <gp4/gp4.h>
#include <gp4/flags.h>

// Low 32 bits of the signed 64-bit product shifted right (imul; shrd eax, edx, n).
static inline int32_t mulShr14(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> 14));
}
static inline int32_t mulShr13(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> 13));
}

// One component of the bilinear blend over the four corner vectors at 0x9351fc (+0x00, +0x10,
// +0x20, +0x30; component offset c): lerp(lerp(B, C, u), lerp(A, D, u), v) in Q14, times 8.
static inline int32_t blend(const int32_t* box, int c, int32_t u, int32_t v) {
    const int32_t first = mulShr14(box[(0x20 + c) / 4] - box[(0x10 + c) / 4], u) + box[(0x10 + c) / 4];
    const int32_t second = mulShr14(box[(0x30 + c) / 4] - box[c / 4], u) + box[c / 4];
    return (mulShr14(second - first, v) + first) << 3;
}

static inline uint32_t nextRandom() {
    gp4::Regs c = {};
    gp4::call_regs(0x0054ff84, c);
    return c.eax;
}

// FUNCTION: GP4 0x00550798
// Initialises the 0x34-byte record at esi (callers walk the array at 0x93523c): a random point of
// the box at 0x9351fc (two random Q14 weights), rotated by the cosine/sine pair 0x8755dc/0x8755d8
// and offset by 0x7a8dd4/d8/dc into +4/+8/+0xc (the blend is kept in 0x934eb8/bc/c0); +0x24 is
// the word +6 of the 0x160-byte ring record reached from [0x679d20] by a cosine-scaled number of
// records (clamped/wrapped by the lap and pit pointers); +0x14 = [0x934ef0]; +0x28 a random
// spread from 0x934f2c/0x934f30 (scaled 2^-13 when byte 0x7ad844 is 0xff, else 2^-14); +0x30 the
// low two random bits and +0x2c a grey colour word. pushal/popal restore every GPR; the flags
// are those of the final `or ebx, eax`.
void __cdecl FUN_00550798(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t u = static_cast<int32_t>((nextRandom() & 0xffu) << 6);
    const int32_t v = static_cast<int32_t>((nextRandom() & 0xffu) << 6);
    const int32_t* const box = GP4_ARRAY(int32_t, 0x009351fc);

    const int32_t bx = blend(box, 0x0, u, v);
    GP4_GLOBAL(int32_t, 0x00934eb8) = bx;
    const int32_t by = blend(box, 0x4, u, v);
    GP4_GLOBAL(int32_t, 0x00934ebc) = by;
    const int32_t bz = blend(box, 0x8, u, v);
    GP4_GLOBAL(int32_t, 0x00934ec0) = bz;

    const int32_t sinA = GP4_GLOBAL(int32_t, 0x008755d8);
    const int32_t cosA = GP4_GLOBAL(int32_t, 0x008755dc);
    *reinterpret_cast<int32_t*>(rec + 0x04) = mulShr14(bx, cosA) + mulShr14(bz, sinA) + GP4_GLOBAL(int32_t, 0x007a8dd4);
    *reinterpret_cast<int32_t*>(rec + 0x0c) = mulShr14(bz, cosA) - mulShr14(bx, sinA) + GP4_GLOBAL(int32_t, 0x007a8ddc);
    *reinterpret_cast<int32_t*>(rec + 0x08) =
        by + GP4_GLOBAL(int32_t, 0x007a8dd8) - mulShr14(bz, GP4_GLOBAL(int32_t, 0x00875240));

    uint32_t seg = GP4_GLOBAL(uint32_t, 0x00679d20);
    uint16_t delta = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00679b5c) -
                                           *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(seg)));
    if (static_cast<int16_t>(delta) < 0) {
        delta = static_cast<uint16_t>(0u - delta);
    }
    const uint32_t cosOffset = static_cast<uint32_t>(delta >> 2) & 0xfffeu;
    const int16_t cosine = *reinterpret_cast<const int16_t*>(GP4_ARRAY(uint8_t, 0x007c5c5c) + cosOffset);
    const int32_t steps = mulShr14(cosine, bz) >> 13;
    int32_t span = static_cast<int32_t>(static_cast<uint32_t>(steps) * 0x160u);

    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
    if (span < 0) {
        const uint32_t back = 0u - static_cast<uint32_t>(span);
        if (seg < pitStart) {
            seg -= back;
            if (seg < GP4_GLOBAL(uint32_t, 0x00891504)) {
                seg += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            seg -= back;
            if (seg < GP4_GLOBAL(uint32_t, 0x00891500)) {
                seg = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
    } else {
        if (seg < pitStart) {
            seg += static_cast<uint32_t>(span);
            if (seg >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                seg -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            seg += static_cast<uint32_t>(span);
            if (seg >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                seg = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
    }
    *reinterpret_cast<int32_t*>(rec + 0x24) =
        static_cast<int32_t>(*reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(seg + 6u))) << 3;
    *reinterpret_cast<uint32_t*>(rec + 0x14) = GP4_GLOBAL(uint32_t, 0x00934ef0);

    const int32_t ra = static_cast<int32_t>(nextRandom() & 0xffu) - 0x80;
    const int32_t rb = static_cast<int32_t>(nextRandom() & 0xffu) - 0x80;
    int32_t spread;
    if (GP4_GLOBAL(uint8_t, 0x007ad844) == 0xffu) {
        const int32_t across = static_cast<int32_t>(static_cast<uint32_t>(mulShr13(rb, GP4_GLOBAL(int32_t, 0x00934f30))) *
                                                    GP4_GLOBAL(uint32_t, 0x008751c4));
        spread = mulShr13(ra, GP4_GLOBAL(int32_t, 0x00934f2c)) + across;
    } else {
        const int32_t across = static_cast<int32_t>(static_cast<uint32_t>(mulShr14(rb, GP4_GLOBAL(int32_t, 0x00934f30))) *
                                                    GP4_GLOBAL(uint32_t, 0x008751c4));
        spread = mulShr14(ra, GP4_GLOBAL(int32_t, 0x00934f2c)) + across;
    }
    *reinterpret_cast<int32_t*>(rec + 0x28) = spread;

    const uint32_t bits = nextRandom();
    *reinterpret_cast<uint32_t*>(rec + 0x30) = bits & 3u;
    const uint32_t grey = ((bits & 0xffu) | 0x30u) & 0x7fu;
    const uint32_t colour = grey | (grey << 8) | (grey << 16) | (((grey << 24) & 0x7f000000u) | 0x3f000000u);
    *reinterpret_cast<uint32_t*>(rec + 0x2c) = colour;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(colour));
}
GP4_IMPL(0x00550798, FUN_00550798, "regs(esi:ptr[0x3c]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+box_00550798")
