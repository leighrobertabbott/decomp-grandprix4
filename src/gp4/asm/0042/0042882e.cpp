#include <gp4/gp4.h>

// Arithmetic flags of a byte-sized OR/AND: CF = OF = AF = 0, ZF/SF/PF from the byte result.
static inline uint32_t logicFlags8(uint8_t value) {
    uint32_t p = value;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return ((p & 1u) == 0u ? 0x04u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// imul + shrd eax, edx, shift: the low 32 bits of the signed 64-bit product shifted right.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// Clamp the magnitude of a signed value to an (unsigned compared) limit, keeping the sign.
static inline uint32_t clampMagnitude(uint32_t value, uint32_t limit) {
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= limit) magnitude = limit;
        return 0u - magnitude;
    }
    if (value >= limit) value = limit;
    return value;
}

// FUNCTION: GP4 0x0042882e
void __cdecl FUN_0042882e(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    const bool flagE5 = (car[0xe5] & 0x80u) != 0u;
    uint32_t eax = flagE5 ? GP4_GLOBAL(uint32_t, 0x00625898) : GP4_GLOBAL(uint32_t, 0x00625894);
    if ((other[0x51] & 0x20u) != 0u
        && ((other[0x51] & 0x08u) != 0u) == ((car[0x71] & 0x08u) != 0u)) {
        const uint32_t base = flagE5 ? GP4_GLOBAL(uint32_t, 0x006258a0) : GP4_GLOBAL(uint32_t, 0x0062589c);
        int32_t weight = GP4_FIELD(int32_t, car, 0x1a0);
        if (weight < 0) weight = 0;
        eax = mulShift(static_cast<int32_t>(eax - base), weight, 0xe) + base;
    }

    gp4::Regs sub = *r;
    sub.eax = eax;
    gp4::call_regs(0x004261cc, sub);
    eax = sub.eax;
    const uint32_t factor = sub.edx;

    eax = mulShift(static_cast<int32_t>(eax), static_cast<int32_t>(factor), 0xe);
    const uint32_t limit = mulShift(static_cast<int32_t>(eax), GP4_FIELD(int32_t, car, 0x284), 0xe);

    int32_t remainder;
    uint32_t value;
    const int32_t b4 = GP4_FIELD(int16_t, car, 0xb4);
    if ((car[0xca] & 0x04u) == 0u) {
        value = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(b4) << 24,
                                                  GP4_GLOBAL(int32_t, 0x00603854), &remainder));
    } else {
        int32_t d = GP4_FIELD(int32_t, car, 0x98);
        if ((car[0x71] & 0x04u) != 0u) d = static_cast<int32_t>(0u - static_cast<uint32_t>(d));
        value = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(b4) * static_cast<int64_t>(d),
                                                  GP4_GLOBAL(int32_t, 0x00601334), &remainder));
    }
    value = clampMagnitude(value, limit);

    const int32_t speed = GP4_FIELD(int32_t, car, 0x98) >> 8;
    const uint32_t speedMag = speed < 0 ? 0u - static_cast<uint32_t>(speed) : static_cast<uint32_t>(speed);
    uint32_t limit2 = mulShift(static_cast<int32_t>(speedMag), GP4_GLOBAL(int32_t, 0x00603c24), 0xe);
    if (static_cast<int32_t>(limit2) < 0) limit2 = 0u - limit2;
    value = clampMagnitude(value, limit2);

    GP4_FIELD(uint32_t, car, 0xac) = value;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value))
                          * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    const uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
    const uint16_t angle = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x18) + static_cast<uint16_t>(eax));
    GP4_FIELD(uint16_t, car, 0x18) = angle;
    eax = (eax & 0xffff0000u) | angle;
    GP4_FIELD(uint16_t, car, 0x0e) = angle;

    bool set = false;
    if ((car[0x8d] & 0x02u) == 0u) {
        const uint32_t raw = GP4_FIELD(uint32_t, car, 0x288);
        eax = static_cast<int32_t>(raw) < 0 ? 0u - raw : raw;
        if (static_cast<int32_t>(eax) > static_cast<int32_t>(limit)) set = true;
    }
    const uint8_t flags = set ? static_cast<uint8_t>(car[0xe5] | 0xe0u) : static_cast<uint8_t>(car[0xe5] & 0x1fu);
    car[0xe5] = flags;

    r->eax = eax;
    r->edx = edx;
    r->ebp = limit;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(flags);
}
GP4_IMPL(0x0042882e, FUN_0042882e, "regs(esi:ptr[0x290]:layout=rec_0042882e, edi:ptr[0x60]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042882e")
