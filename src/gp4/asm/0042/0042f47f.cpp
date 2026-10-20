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

static inline uint16_t word16(const uint8_t* p, uint32_t offset) {
    return *reinterpret_cast<const uint16_t*>(p + offset);
}

static inline uint32_t& dword32(uint8_t* p, uint32_t offset) {
    return *reinterpret_cast<uint32_t*>(p + offset);
}

// 0x42f6cc: clears bit 3 of +0x161; when bit 7 was set, clears it and the due time +0x17c.
static inline void cancel(uint8_t* car, gp4::Regs& s) {
    car[0x161] = static_cast<uint8_t>(car[0x161] & 0xf7u);
    setFlags(s, logicFlags8(car[0x161] & 0x80u));
    if ((car[0x161] & 0x80u) != 0u) {
        dword32(car, 0x17c) = 0;
        car[0x161] = static_cast<uint8_t>(car[0x161] & 0x7fu);
        setFlags(s, logicFlags8(car[0x161]));
    }
}

// The due time +0x17c is set (nonzero) and reached by the clock [0x66d814] (sign of clock - due).
static inline bool dueReached(uint8_t* car, gp4::Regs& s) {
    const uint32_t due = dword32(car, 0x17c);
    setFlags(s, logicFlags32(due));
    if (due == 0u) return false;
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    setFlags(s, addFlags32(0u - due, clock));
    return ((clock - due) & 0x80000000u) == 0u;
}

// 0x42f3c7, then a new due time: clock + random byte * 4 + 500.
static inline void steerAndReschedule(uint8_t* car, gp4::Regs& s) {
    gp4::call_regs(0x0042f3c7, s);
    gp4::call_regs(0x00401462, s);
    s.eax = ((s.eax & 0xffu) << 2) + 0x1f4u;
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    dword32(car, 0x17c) = clock;
    dword32(car, 0x17c) = clock + s.eax;
    setFlags(s, addFlags32(clock, s.eax));
}

// Car in esi against the other car [0x6043e4]. With bit 7 of +0x161 set (and the segment +0x1c not a
// bit-5-only segment, the +0x9a difference below [0x603b80]) it re-runs 0x42f3c7 whenever the due time
// +0x17c is reached. Otherwise a reached due time with bit 3 sets bit 7 and does the same; else, on a
// plain segment with word +0x100 below [0x62ac54], when the other car is the car two places ahead in
// byte +0x415, its +0x68 lead is 0..[0x603b12), the lateral gap (+0xe2 or 0x40dc8a) lies within
// [0x603b7c, 0x603b7a] and the +0x9a difference is below [0x603b7e], a speed term from segment word +0xa6,
// [0x62ac50] and +0x9a in [[0x603bb0], [0x603bac]) sets bit 3, adds [0x6223ec] through 0x42a0cf and sets the
// due time clock + random byte * 8. Every failed test clears bit 3 (and bit 7 with the due time).
// FUNCTION: GP4 0x0042f47f
void __cdecl FUN_0042f47f(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    setFlags(s, logicFlags8(car[0xfe] & 0x40u));
    if ((car[0xfe] & 0x40u) != 0u) {
        cancel(car, s);
    } else if (setFlags(s, logicFlags8(car[0xcc] & 0x10u)), (car[0xcc] & 0x10u) != 0u) {
        cancel(car, s);
    } else {
        s.edi = dword32(car, 0x1c);
        const uint8_t* segment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(s.edi));
        setFlags(s, logicFlags8(car[0x161] & 0x80u));
        if ((car[0x161] & 0x80u) != 0u) {
            bool keep = true;
            if ((segment[0x51] & 0x20u) != 0u && (segment[0x51] & 0x80u) == 0u) {
                keep = false;
            }
            if (keep) {
                s.edx = GP4_GLOBAL(uint32_t, 0x006043e4);
                const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(s.edx));
                const uint16_t gap = static_cast<uint16_t>(word16(car, 0x9a) - word16(other, 0x9a));
                s.eax = (s.eax & 0xffff0000u) | gap;
                if (static_cast<int16_t>(gap) >= GP4_GLOBAL(int16_t, 0x00603b80)) {
                    keep = false;
                }
            }
            if (!keep) {
                cancel(car, s);
            } else if (dueReached(car, s)) {
                steerAndReschedule(car, s);
            }
        } else {
            setFlags(s, logicFlags8(car[0x161] & 8u));
            if ((car[0x161] & 8u) != 0u && dueReached(car, s)) {
                car[0x161] = static_cast<uint8_t>(car[0x161] | 0x80u);
                steerAndReschedule(car, s);
            } else {
                bool keep = (segment[0x51] & 0x20u) == 0u;
                if (keep) {
                    s.eax = word16(car, 0x100);
                    keep = static_cast<int32_t>(s.eax) < GP4_GLOBAL(int32_t, 0x0062ac54);
                }
                const uint8_t* other = nullptr;
                if (keep) {
                    const uint8_t place = static_cast<uint8_t>(car[0x415] + 2u);
                    s.eax = (s.eax & 0xffffff00u) | place;
                    s.edx = GP4_GLOBAL(uint32_t, 0x006043e4);
                    other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(s.edx));
                    keep = place == other[0x415];
                }
                if (keep) {
                    const uint16_t lead = static_cast<uint16_t>(word16(car, 0x68) - word16(other, 0x68));
                    s.eax = (s.eax & 0xffff0000u) | lead;
                    keep = static_cast<int16_t>(lead) >= 0 && static_cast<int16_t>(lead) < GP4_GLOBAL(int16_t, 0x00603b12);
                }
                if (keep) {
                    uint16_t lateral;
                    if ((car[0x9c] & 2u) != 0u || (other[0x9c] & 2u) != 0u) {
                        const uint32_t savedEbx = s.ebx;
                        s.ebx = s.edx;
                        gp4::call_regs(0x0040dc8a, s);
                        s.ebx = savedEbx;
                        lateral = static_cast<uint16_t>(s.eax);
                    } else {
                        lateral = static_cast<uint16_t>(word16(other, 0xe2) - word16(car, 0xe2));
                    }
                    lateral = static_cast<uint16_t>(0u - lateral);
                    s.eax = (s.eax & 0xffff0000u) | lateral;
                    keep = static_cast<int16_t>(lateral) <= GP4_GLOBAL(int16_t, 0x00603b7a)
                        && static_cast<int16_t>(lateral) >= GP4_GLOBAL(int16_t, 0x00603b7c);
                }
                if (keep) {
                    const uint16_t gap = static_cast<uint16_t>(word16(car, 0x9a) - word16(other, 0x9a));
                    s.eax = (s.eax & 0xffff0000u) | gap;
                    keep = static_cast<int16_t>(gap) < GP4_GLOBAL(int16_t, 0x00603b7e);
                }
                if (keep) {
                    const uint8_t* seg = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(dword32(car, 0x1c)));
                    int32_t limit = static_cast<int32_t>(word16(seg, 0xa6));
                    int32_t top = GP4_GLOBAL(int32_t, 0x0062ac50);
                    if (top > limit) {
                        top = limit;
                    }
                    const int32_t speed = static_cast<int16_t>(word16(car, 0x9a));
                    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(top) - static_cast<uint32_t>(speed)))
                        * static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(speed) + static_cast<uint32_t>(speed)));
                    int32_t term = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24));
                    s.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
                    s.ebp = static_cast<uint32_t>(speed) + static_cast<uint32_t>(speed);
                    if (term < 0) {
                        term = 0;
                    }
                    if (limit > GP4_GLOBAL(int32_t, 0x0062ac50)) {
                        limit = static_cast<int32_t>(static_cast<uint32_t>(limit) - GP4_GLOBAL(uint32_t, 0x0062ac50));
                        term = static_cast<int32_t>(static_cast<uint32_t>(term) + static_cast<uint32_t>(limit));
                    }
                    s.ecx = static_cast<uint32_t>(limit);
                    s.eax = static_cast<uint32_t>(term);
                    keep = term < GP4_GLOBAL(int32_t, 0x00603bac) && term >= GP4_GLOBAL(int32_t, 0x00603bb0);
                }
                if (!keep) {
                    cancel(car, s);
                } else {
                    setFlags(s, logicFlags8(car[0x161] & 8u));
                    if ((car[0x161] & 8u) == 0u) {
                        car[0x161] = static_cast<uint8_t>(car[0x161] | 8u);
                        s.eax = GP4_GLOBAL(uint32_t, 0x006223ec);
                        gp4::call_regs(0x0042a0cf, s);
                        gp4::call_regs(0x00401462, s);
                        s.eax = (s.eax & 0xffu) << 3;
                        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                        dword32(car, 0x17c) = clock;
                        dword32(car, 0x17c) = clock + s.eax;
                        setFlags(s, addFlags32(clock, s.eax));
                    }
                }
            }
        }
    }

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->edi = s.edi;
    r->eflags = s.eflags;
}
GP4_IMPL(0x0042f47f, FUN_0042f47f, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, edi:u32, esi:ptr[0x420]:layout=car_0042ba87+car_0040ea09+car_00428bea+car_0042f47f) -> (eax, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+cars_0066fef0+list_0066fe3c+pair_0042e8e6+scan_00409aec+state_0042f3c7+cars_0042f47f")
