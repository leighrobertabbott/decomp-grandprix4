#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result or `cmp byte, 0` (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Flags of `or reg32, reg32`.
static inline uint32_t orFlags32(uint32_t value) {
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~0x8d5u) | flags;
}

static inline bool carry(const gp4::Regs& s) {
    return (s.eflags & 1u) != 0u;
}

// Settles the car's position (eax, edx = x, z; edi = a track segment) on the segment ring and
// records the result: [0x6278c0] / [0x6278c8] = position, [0x627946] = 0 (0xff when the car sits
// on a pit-lane entry found by 0x00406b0a), and [0x6278cc] / [0x6278d0] = the resulting eax / edi
// (the piece of track the car is on and its record). Each lookup is 0x00406ac7 (nearest record in
// the main ring) or 0x00406a57 (pit window [0x866ed0, 0x866ed4)), followed by 0x004067b7 for slot
// ebx = 0 / 1; success picks the pit-lane pair 0x629f08 / 0x629f10, failure the main pair
// 0x629f04 / 0x629f0c (and tries 0x00406b0a when [0x627945] is set). ebx is preserved.
// FUNCTION: GP4 0x00406dbb
void __cdecl FUN_00406dbb(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x006278c0) = s.eax;
    GP4_GLOBAL(uint32_t, 0x006278c8) = s.edx;
    GP4_GLOBAL(uint8_t, 0x00627946) = 0;

    bool inside = false;
    setFlags(s, cmpFlags32(s.edi, GP4_GLOBAL(uint32_t, 0x00891500)));
    if (s.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        setFlags(s, cmpFlags32(s.edi, GP4_GLOBAL(uint32_t, 0x00866ed0)));
        if (s.edi >= GP4_GLOBAL(uint32_t, 0x00866ed0)) {
            setFlags(s, cmpFlags32(s.edi, GP4_GLOBAL(uint32_t, 0x00866ed4)));
            inside = s.edi < GP4_GLOBAL(uint32_t, 0x00866ed4);
        }
        if (!inside) {
            s.edi = GP4_FIELD(uint32_t, s.edi, 0x44);
        }
    }

    bool found = false;
    if (!inside) {
        gp4::call_regs(0x00406ac7, s);
        s.ebx = 0;
        gp4::call_regs(0x004067b7, s);
        if (carry(s)) {
            s.edi = GP4_FIELD(uint32_t, s.edi, 0x44);
            setFlags(s, orFlags32(s.edi));
            if (s.edi != 0u) {
                gp4::call_regs(0x00406a57, s);
                if (carry(s)) {
                    s.ebx = 1;
                    gp4::call_regs(0x004067b7, s);
                    if (!carry(s)) {
                        found = true;
                    } else {
                        s.edx = s.eax;
                        const uint32_t dl = (s.edx ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0xffu;
                        s.edx = (s.edx & 0xffffff00u) | dl;
                        setFlags(s, logicFlags8(dl));
                        if ((dl & 0x80u) == 0u) {
                            gp4::call_regs(0x00406d90, s);
                            found = carry(s);
                        }
                    }
                }
            }
        }
    } else {
        gp4::call_regs(0x00406a57, s);
        if (carry(s)) {
            s.ebx = 1;
            gp4::call_regs(0x004067b7, s);
            if (!carry(s)) {
                found = true;
            } else {
                s.edi = GP4_FIELD(uint32_t, s.edi, 0x44);
                gp4::call_regs(0x00406ac7, s);
                s.ebx = 0;
                gp4::call_regs(0x004067b7, s);
                found = carry(s);
            }
        } else {
            setFlags(s, cmpFlags32(s.edi, GP4_GLOBAL(uint32_t, 0x00891500)));
            if (s.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
                s.edi = GP4_FIELD(uint32_t, s.edi, 0x44);
            }
            gp4::call_regs(0x00406ac7, s);
            s.ebx = 0;
            gp4::call_regs(0x004067b7, s);
        }
    }

    if (found) {
        s.edi = GP4_GLOBAL(uint32_t, 0x00629f10);
        s.eax = GP4_GLOBAL(uint32_t, 0x00629f08);
    } else {
        s.edi = GP4_GLOBAL(uint32_t, 0x00629f0c);
        s.eax = GP4_GLOBAL(uint32_t, 0x00629f04);
        const uint32_t pitFlag = GP4_GLOBAL(uint8_t, 0x00627945);
        setFlags(s, logicFlags8(pitFlag));
        if (pitFlag != 0u) {
            gp4::call_regs(0x00406b0a, s);
            if (carry(s)) {
                GP4_GLOBAL(uint8_t, 0x00627946) = 0xff;
                const uint32_t pitLane = GP4_GLOBAL(uint8_t, 0x00627944);
                setFlags(s, logicFlags8(pitLane));
                if (pitLane != 0u) {
                    s.edi = GP4_GLOBAL(uint32_t, 0x00629f10);
                    s.eax = GP4_GLOBAL(uint32_t, 0x00629f08);
                }
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x006278cc) = s.eax;
    GP4_GLOBAL(uint32_t, 0x006278d0) = s.edi;

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00406dbb, FUN_00406dbb, "regs(eax:u32, edx:u32, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); cases=1000; globals=fuzz:i32+layout=state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab")
