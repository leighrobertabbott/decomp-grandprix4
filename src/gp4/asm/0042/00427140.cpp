#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Byte offset into the Q14 cosine table 0x007c5c5c for a 16-bit angle: |angle| >> 2, word aligned.
static inline uint32_t tableOffset(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    return static_cast<uint32_t>(static_cast<uint16_t>(angle >> 2)) & 0xfffeu;
}

static inline int32_t tableWord(uint32_t byteOffset) {
    return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + byteOffset));
}

// FUNCTION: GP4 0x00427140
// Car esi: the distance D = ((([esi+0x98] - [0x603d38]) >> 1) + [0x603d38]) * [0x603854] >> 24 (negated,
// with the high dword of the product, when bit 2 of byte +0x71 is set; that high dword's low word goes
// to +0xde) is stored to [0x603d30] and the vector (0, D) is rotated by the heading word [0x603d6a]
// through the cosine table (cos -> [0x934de8], -sin -> [0x934dec]) and offset by [0x603d28]/[0x603d2c]
// into the point [0x623e30]/[0x623e38]. With bit 2 set the record ring walk 0x004061ed (point >> 11)
// finds the record; otherwise up to 10 forward steps from the record edi, each taken while 0x00405e19
// says the point is not behind the next record, stepping through 0x00405f9e on flagged records and
// advancing +0x1b8 (or copying the record into it) and storing the record in +0x1c. 0x0040460a then
// maps the point into the record frame: words +0x92 = y >> 8, +0x146 = max(0x400 - (w14 * (x >> 8) >>
// 14), 0x10) and +0x96 = +0x92 + (w14 * (x >> 8) >> 15), w14 = word +0x14 of the record (0: no product).
// The flags are those of the final 16-bit add.
void __cdecl FUN_00427140(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const bool reverse = (GP4_FIELD(uint8_t, car, 0x71) & 4u) != 0u;

    const uint32_t previous = GP4_GLOBAL(uint32_t, 0x00603d38);
    const uint32_t mean = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(uint32_t, car, 0x98) - previous) >> 1) + previous;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(mean)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    uint32_t distance = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
    uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    if (reverse) {
        distance = 0u - distance;
        high = 0u - high;
    }
    GP4_GLOBAL(uint32_t, 0x00603d30) = distance;
    GP4_FIELD(uint16_t, car, 0xde) = static_cast<uint16_t>(high);

    const uint16_t heading = GP4_GLOBAL(uint16_t, 0x00603d6a);
    const int32_t cosine = tableWord(tableOffset(heading));
    GP4_GLOBAL(int32_t, 0x00934de8) = cosine;
    const int32_t negSine = 0 - tableWord(tableOffset(static_cast<uint16_t>(0x4000u - heading)));
    GP4_GLOBAL(int32_t, 0x00934dec) = negSine;

    const int32_t d = static_cast<int32_t>(distance);
    const uint32_t x = (0u - mulShr14(d, negSine)) + GP4_GLOBAL(uint32_t, 0x00603d28);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x;
    const uint32_t z = mulShr14(d, cosine) + GP4_GLOBAL(uint32_t, 0x00603d2c);
    GP4_GLOBAL(uint32_t, 0x00623e38) = z;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;
    s.eax = 0;
    s.edx = 0;
    s.ebx = x;
    s.ecx = z;

    uint32_t record = r->edi;
    if (reverse) {
        GP4_GLOBAL(int32_t, 0x00623e30) = static_cast<int32_t>(x) >> 11;
        GP4_GLOBAL(int32_t, 0x00623e38) = static_cast<int32_t>(z) >> 11;
        s.edi = record;
        gp4::call_regs(0x004061ed, s);
        GP4_GLOBAL(uint32_t, 0x00623e38) = z;
        GP4_GLOBAL(uint32_t, 0x00623e30) = x;
        record = s.edx;
        GP4_FIELD(uint32_t, car, 0x1c) = record;
        GP4_FIELD(uint32_t, car, 0x1b8) = record;
    } else {
        uint32_t count = 10;
        for (;;) {
            uint32_t next = record + 0x160u;
            if (next >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                if (next < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    next -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                } else if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    next = GP4_GLOBAL(uint32_t, 0x00866f04);
                }
            }
            s.ecx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e30) >> 11);
            s.ebx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e38) >> 11);
            s.edi = next;
            s.ebp = count;
            gp4::call_regs(0x00405e19, s);
            s.edi = record;
            s.ebp = count;
            if ((s.eflags & 1u) != 0u) {
                break;
            }

            record += 0x160u;
            if ((GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record)), 0x92) & 8u) != 0u) {
                s.edi = record;
                gp4::call_regs(0x00405f9e, s);
                record = s.edi;
                count = s.ebp;
            }

            if ((GP4_FIELD(uint8_t, car, 0x162) & 8u) != 0u) {
                uint32_t ahead = GP4_FIELD(uint32_t, car, 0x1b8) + 0x160u;
                if (ahead >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    if (ahead < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        ahead -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                    } else if (ahead >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                        ahead = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                    }
                }
                GP4_FIELD(uint32_t, car, 0x1b8) = ahead;
                s.eax = ahead;
            } else {
                GP4_FIELD(uint32_t, car, 0x1b8) = record;
            }
            GP4_FIELD(uint32_t, car, 0x1c) = record;

            if (--count == 0u) {
                break;
            }
        }
        s.ebp = count;
    }

    s.edi = record;
    gp4::call_regs(0x0040460a, s);

    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
    const int32_t y8 = GP4_GLOBAL(int32_t, 0x00934df4) >> 8;
    GP4_FIELD(uint16_t, car, 0x92) = static_cast<uint16_t>(y8);
    const int32_t x8 = GP4_GLOBAL(int32_t, 0x00934df0) >> 8;
    uint32_t edx = s.edx;

    uint16_t ax = GP4_FIELD(uint16_t, rec, 0x14);
    if (ax != 0u) {
        const int32_t lateral = static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(static_cast<int16_t>(x8));
        ax = static_cast<uint16_t>(lateral >> 14);
        edx = (edx & 0xffff0000u) | ax;
    }
    ax = static_cast<uint16_t>(0x400u - ax);
    if (static_cast<int16_t>(ax) < 0x10) {
        ax = 0x10u;
    }
    GP4_FIELD(uint16_t, car, 0x146) = ax;

    ax = GP4_FIELD(uint16_t, rec, 0x14);
    if (ax != 0u) {
        const int32_t lateral = static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(static_cast<int16_t>(x8));
        ax = static_cast<uint16_t>(lateral >> 15);
        edx = (edx & 0xffff0000u) | ax;
    }
    const uint16_t base = GP4_FIELD(uint16_t, car, 0x92);
    const uint16_t sum = static_cast<uint16_t>(ax + base);
    const uint32_t flags = addFlags16(ax, base);
    GP4_FIELD(uint16_t, car, 0x96) = sum;

    r->eax = (static_cast<uint32_t>(y8) & 0xffff0000u) | sum;
    r->ebx = static_cast<uint32_t>(x8);
    r->ecx = s.ecx;
    r->edx = edx;
    r->edi = record;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00427140, FUN_00427140, "regs(esi:ptr[0x420]:layout=car_004129fa+car_0042b17e, edi:rec[0x8943a0,0x160,0..11], ebp:u32) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00405489+state_00403800+ranges_004322a6+ring_004317ae+ring_00405a12+links_00405a12+state_004059ef+div_00403bb6+state_00427140")
