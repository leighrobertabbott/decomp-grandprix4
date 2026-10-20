#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST/OR/AND result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x040u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit TEST result: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return byteParity(result) | (result == 0u ? 0x040u : 0u) | ((result & 0x80000000u) != 0u ? 0x080u : 0u);
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~0x8d5u) | flags;
}

// 0x441045: with +0xce bit 1 and +0x161 bit 1 set, 0x434d4c runs; then, unless +0x7b bit 1 / bit 6 or
// +0x161 bit 6 is set, sets +0x7b bit 1 and +0xfe bit 5.
static inline void stage1045(uint8_t* car, gp4::Regs& s) {
    setFlags(s, logicFlags8(car[0xce] & 2u));
    if ((car[0xce] & 2u) == 0u) return;
    setFlags(s, logicFlags8(car[0x161] & 2u));
    if ((car[0x161] & 2u) == 0u) return;
    gp4::call_regs(0x00434d4c, s);
    setFlags(s, logicFlags8(car[0x7b] & 2u));
    if ((car[0x7b] & 2u) != 0u) return;
    setFlags(s, logicFlags8(car[0x7b] & 0x40u));
    if ((car[0x7b] & 0x40u) != 0u) return;
    setFlags(s, logicFlags8(car[0x161] & 0x40u));
    if ((car[0x161] & 0x40u) != 0u) return;
    car[0x7b] = static_cast<uint8_t>(car[0x7b] | 2u);
    car[0xfe] = static_cast<uint8_t>(car[0xfe] | 0x20u);
    setFlags(s, logicFlags8(car[0xfe]));
}

// 0x441023: with +0xe5 bit 2 and +0xce bit 1 set, 0x434d11 runs when +0x8d bit 1 is clear, else stage 0x441045.
static inline void stage1023(uint8_t* car, gp4::Regs& s) {
    setFlags(s, logicFlags8(car[0xe5] & 4u));
    if ((car[0xe5] & 4u) == 0u) return;
    setFlags(s, logicFlags8(car[0xce] & 2u));
    if ((car[0xce] & 2u) == 0u) return;
    setFlags(s, logicFlags8(car[0x8d] & 2u));
    if ((car[0x8d] & 2u) == 0u) {
        gp4::call_regs(0x00434d11, s);
        return;
    }
    stage1045(car, s);
}

// Car-state step for the record in esi (only while byte +0x161 has bit 2). Without +0x8d bit 4 (and bit 1),
// when +0xcd bit 2 is set or byte +0x70 equals +0x9e and byte +0x9f * 8 is at most the low 11 bits of word
// +0x1a of the segment at +0x1c, it sets +0x8d bit 4 (and +0x82, +0x203 bits, clears +0xce bit 1), runs the
// five mode routines 0x440cc8/0x440ce9/0x440d16/0x440d69/0x440d89 and marks bytes by the mode +0x84. With
// +0x8d bit 4 it waits for the due clock +0x1e4 ([0x66d814] - due >= 0), reschedules it once per +0x87 bit
// 1/2 request (5000 + random byte * 128), else sets +0xce bit 1 (0x418097 when +0xce bit 3) and runs the
// 0x434d11 / 0x434d4c stages. eax/ecx/edx and the flags are those of the last instruction or routine.
// FUNCTION: GP4 0x00440e25
void __cdecl FUN_00440e25(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    setFlags(s, logicFlags8(car[0x161] & 4u));
    if ((car[0x161] & 4u) != 0u) {
        setFlags(s, logicFlags8(car[0x8d] & 0x10u));
        if ((car[0x8d] & 0x10u) == 0u) {
            setFlags(s, logicFlags8(car[0x8d] & 2u));
            bool start = (car[0x8d] & 2u) == 0u;
            if (start) {
                setFlags(s, logicFlags8(car[0xcd] & 4u));
                if ((car[0xcd] & 4u) == 0u) {
                    const uint8_t lane = car[0x70];
                    s.eax = (s.eax & 0xffffff00u) | lane;
                    setFlags(s, cmpFlags8(lane, car[0x9e]));
                    if (lane != car[0x9e]) {
                        start = false;
                    } else {
                        s.eax = static_cast<uint32_t>(car[0x9f]) << 3;
                        const uint8_t* segment = reinterpret_cast<const uint8_t*>(
                            static_cast<uintptr_t>(*reinterpret_cast<const uint32_t*>(car + 0x1c)));
                        s.edx = static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(segment + 0x1a)) & 0x7ffu;
                        setFlags(s, cmpFlags32(s.eax, s.edx));
                        if (static_cast<int32_t>(s.eax) > static_cast<int32_t>(s.edx)) {
                            start = false;
                        }
                    }
                }
            }
            if (start) {
                car[0x8d] = static_cast<uint8_t>(car[0x8d] | 0x10u);
                car[0x82] = static_cast<uint8_t>(car[0x82] | 0x40u);
                car[0x82] = static_cast<uint8_t>(car[0x82] | 1u);
                car[0xce] = static_cast<uint8_t>(car[0xce] & 0xfdu);
                car[0x203] = static_cast<uint8_t>(car[0x203] | 0x10u);
                setFlags(s, logicFlags8(car[0x203]));
                gp4::call_regs(0x00440cc8, s);
                gp4::call_regs(0x00440ce9, s);
                gp4::call_regs(0x00440d16, s);
                gp4::call_regs(0x00440d69, s);
                gp4::call_regs(0x00440d89, s);

                const uint8_t mode = car[0x84];
                s.eax = (s.eax & 0xffffff00u) | mode;
                if (mode == 3u) {
                    car[0xce] = static_cast<uint8_t>(car[0xce] | 8u);
                    setFlags(s, logicFlags8(car[0xce]));
                } else if (mode == 8u) {
                    car[0x87] = static_cast<uint8_t>(car[0x87] | 2u);
                    car[0xce] = static_cast<uint8_t>(car[0xce] | 8u);
                    setFlags(s, logicFlags8(car[0xce]));
                } else if (mode == 5u) {
                    car[0x87] = static_cast<uint8_t>(car[0x87] | 4u);
                    car[0xce] = static_cast<uint8_t>(car[0xce] | 8u);
                    setFlags(s, logicFlags8(car[0xce]));
                } else if (mode == 9u || mode == 6u || mode == 7u || mode == 2u || mode == 1u) {
                    car[0x71] = static_cast<uint8_t>(car[0x71] | 0x80u);
                    car[0x230] = static_cast<uint8_t>(car[0x230] & 0x7fu);
                    car[0xfe] = static_cast<uint8_t>(car[0xfe] | 4u);
                    setFlags(s, logicFlags8(car[0xfe]));
                } else if (mode == 0u || mode == 4u) {
                    car[0xce] = static_cast<uint8_t>(car[0xce] | 2u);
                    setFlags(s, logicFlags8(car[0xce]));
                } else {
                    setFlags(s, cmpFlags8(mode, 4u));
                }
            }
        } else {
            setFlags(s, logicFlags8(car[0xce] & 2u));
            if ((car[0xce] & 2u) != 0u) {
                stage1023(car, s);
            } else {
                const uint32_t due = *reinterpret_cast<const uint32_t*>(car + 0x1e4);
                setFlags(s, logicFlags32(due));
                if (due != 0u) {
                    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                    setFlags(s, addFlags32(0u - due, clock));
                    if (((clock - due) & 0x80000000u) == 0u) {
                        bool reschedule = false;
                        setFlags(s, logicFlags8(car[0x87] & 2u));
                        if ((car[0x87] & 2u) != 0u) {
                            setFlags(s, logicFlags8(car[0x87] & 0x20u));
                            if ((car[0x87] & 0x20u) == 0u) {
                                car[0x87] = static_cast<uint8_t>(car[0x87] | 0x20u);
                                reschedule = true;
                            }
                        }
                        if (!reschedule) {
                            setFlags(s, logicFlags8(car[0x87] & 4u));
                            if ((car[0x87] & 4u) != 0u) {
                                setFlags(s, logicFlags8(car[0x87] & 0x40u));
                                if ((car[0x87] & 0x40u) == 0u) {
                                    car[0x87] = static_cast<uint8_t>(car[0x87] | 0x40u);
                                    car[0x87] = static_cast<uint8_t>(car[0x87] | 0x20u);
                                    reschedule = true;
                                }
                            }
                        }
                        if (reschedule) {
                            setFlags(s, logicFlags8(car[0x87]));
                            gp4::call_regs(0x00401462, s);
                            s.eax = ((s.eax & 0xffu) << 7) + 0x1388u;
                            const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
                            *reinterpret_cast<uint32_t*>(car + 0x1e4) = now;
                            *reinterpret_cast<uint32_t*>(car + 0x1e4) = now + s.eax;
                            setFlags(s, addFlags32(now, s.eax));
                        } else {
                            setFlags(s, logicFlags8(car[0x8d] & 2u));
                            if ((car[0x8d] & 2u) != 0u) {
                                stage1045(car, s);
                            } else {
                                car[0xce] = static_cast<uint8_t>(car[0xce] | 2u);
                                setFlags(s, logicFlags8(car[0xce] & 8u));
                                if ((car[0xce] & 8u) != 0u) {
                                    gp4::call_regs(0x00418097, s);
                                }
                                stage1023(car, s);
                            }
                        }
                    }
                }
            }
        }
    }

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->eflags = s.eflags;
}
GP4_IMPL(0x00440e25, FUN_00440e25, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x358]:layout=car_00411109+rec_004105c7+rec_0041b950) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_004105c7")
