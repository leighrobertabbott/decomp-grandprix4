#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Track segment record: the two words read by the lateral-offset test.
struct Unknown_0042fd08_Segment {
    int16_t Unknown00;
    uint8_t Unknown02[0x18 - 0x02];
    int16_t Unknown18;
};

// Fields of the car record (esi) this routine reads / writes.
struct Unknown_0042fd08_Car {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x1c - 0x10];
    Unknown_0042fd08_Segment* Segment1C;
    uint8_t Unknown20[0x90 - 0x20];
    int16_t Unknown90;
    uint8_t Unknown92[0x94 - 0x92];
    int16_t Unknown94;
    uint8_t Unknown96[0x9a - 0x96];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x11c - 0x9c];
    int16_t Unknown11C;
    int16_t Unknown11E;
    uint8_t Unknown120[0x1ce - 0x120];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x230 - 0x1cf];
    uint8_t Flags230;
};

// Leaves ax = word +0x90 +/- 0x200 and dx = the clamp bound; returns whether the adjusted word is
// within the bound (negative word: ax + 0x200 >= min(-word +0x11c, word +0x94); otherwise
// ax - 0x200 <= max(word +0x11e, word +0x94)). `flags` receives the final 16-bit compare.
static inline bool withinBand(const Unknown_0042fd08_Car* car, uint32_t& eax, uint32_t& edx, uint32_t& flags) {
    uint16_t ax = static_cast<uint16_t>(car->Unknown90);
    uint16_t dx;
    bool within;
    if (static_cast<int16_t>(ax) < 0) {
        dx = static_cast<uint16_t>(0u - static_cast<uint16_t>(car->Unknown11C));
        if (static_cast<int16_t>(dx) > car->Unknown94) {
            dx = static_cast<uint16_t>(car->Unknown94);
        }
        ax = static_cast<uint16_t>(ax + 0x200u);
        within = static_cast<int16_t>(ax) >= static_cast<int16_t>(dx);
    } else {
        dx = static_cast<uint16_t>(car->Unknown11E);
        if (static_cast<int16_t>(dx) < car->Unknown94) {
            dx = static_cast<uint16_t>(car->Unknown94);
        }
        ax = static_cast<uint16_t>(ax - 0x200u);
        within = static_cast<int16_t>(ax) <= static_cast<int16_t>(dx);
    }
    flags = cmpFlags16(ax, dx);
    eax = (eax & 0xffff0000u) | ax;
    edx = (edx & 0xffff0000u) | dx;
    return within;
}

// FUNCTION: GP4 0x0042fd08
// Hysteresis for bit 3 of car byte +0x230. Bit 0 of +0x1ce forces it clear. Otherwise the signed
// word +0x90 is tested against a +/-0x200 band around the bounds in words +0x94/+0x11c/+0x11e.
// Clear -> set: outside the band and word +0x9a >= [0x603be8]. Set -> clear: outside the band and
// +0x9a < [0x603be8], or inside the band with |seg[0] + seg[0x18] - word +0xe| < [0x603bf0] or
// +0x9a < [0x603bec] (seg = dword +0x1c). eax, edx (and edi in the set case) carry the last values.
void __cdecl FUN_0042fd08(gp4::Regs* r) {
    Unknown_0042fd08_Car* car = reinterpret_cast<Unknown_0042fd08_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t edi = r->edi;
    uint32_t flags;
    bool clear = false;

    if ((car->Flags1CE & 1u) != 0u) {
        clear = true;
    } else if ((car->Flags230 & 8u) != 0u) {
        if (!withinBand(car, eax, edx, flags)) {
            eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown9A));
            const int32_t limit = GP4_GLOBAL(int32_t, 0x00603be8);
            flags = cmpFlags32(eax, static_cast<uint32_t>(limit));
            clear = static_cast<int32_t>(eax) < limit;
        } else {
            const Unknown_0042fd08_Segment* seg = car->Segment1C;
            edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(seg));
            const uint16_t sum = static_cast<uint16_t>(
                static_cast<uint16_t>(static_cast<uint16_t>(seg->Unknown00) + static_cast<uint16_t>(seg->Unknown18))
                - static_cast<uint16_t>(car->Unknown0E));
            int32_t dist = static_cast<int16_t>(sum);
            if (dist < 0) {
                dist = static_cast<int32_t>(0u - static_cast<uint32_t>(dist));
            }
            eax = static_cast<uint32_t>(dist);
            const int32_t limitDist = GP4_GLOBAL(int32_t, 0x00603bf0);
            flags = cmpFlags32(eax, static_cast<uint32_t>(limitDist));
            if (dist < limitDist) {
                clear = true;
            } else {
                eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown9A));
                const int32_t limitSpeed = GP4_GLOBAL(int32_t, 0x00603bec);
                flags = cmpFlags32(eax, static_cast<uint32_t>(limitSpeed));
                clear = static_cast<int32_t>(eax) < limitSpeed;
            }
        }
    } else {
        if (!withinBand(car, eax, edx, flags)) {
            eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown9A));
            const int32_t limit = GP4_GLOBAL(int32_t, 0x00603be8);
            flags = cmpFlags32(eax, static_cast<uint32_t>(limit));
            if (static_cast<int32_t>(eax) >= limit) {
                car->Flags230 = static_cast<uint8_t>(car->Flags230 | 8u);
                flags = logicFlags8(car->Flags230);
            }
        }
    }

    if (clear) {
        car->Flags230 = static_cast<uint8_t>(car->Flags230 & 0xf7u);
        flags = logicFlags8(car->Flags230);
    }

    r->eax = eax;
    r->edx = edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0042fd08, FUN_0042fd08, "regs(eax:u32, edx:u32, esi:ptr[0x238]:layout=car_0043c677+rec_0042f450) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
