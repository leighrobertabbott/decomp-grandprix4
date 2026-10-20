#include <gp4/gp4.h>
#include <gp4/flags.h>

// Word of the Q14 cosine table at 0x7c5c5c for a 16-bit angle: |angle| >> 3 (signed 16-bit abs).
static inline int32_t cosQ14(uint16_t angle) {
    const uint16_t magnitude = static_cast<int16_t>(angle) < 0 ? static_cast<uint16_t>(0u - angle) : angle;
    const uint32_t offset = static_cast<uint32_t>(magnitude >> 2) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
}

static inline uint32_t abs32(uint32_t v) { return static_cast<int32_t>(v) < 0 ? 0u - v : v; }

// Low dword of the signed 64-bit product a*b shifted right by 14 (imul; shrd eax,edx,0xe).
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Sign-extended value << 11 as the 64-bit dividend of cdq; shld edx,eax,0xb; shl eax,0xb.
static inline int64_t shl11(uint32_t v) {
    return static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(v))) << 11);
}

// FUNCTION: GP4 0x00518f17
// pushal/popal routine on the object at ebx: angle of (x, z) relative to the point
// (0x7a8dd4, 0x7a8ddc) via 0x401315, then the derived scale/distance terms 0x88fce4,
// 0x88fce8, 0x88fc78 and 0x88fc74.
void __cdecl FUN_00518f17(gp4::Regs* r) {
    uint8_t* obj = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t range = GP4_FIELD(uint32_t, obj, 0x8) >> 8;
    const uint32_t heading = GP4_FIELD(uint32_t, obj, 0xc);
    const uint32_t dx = static_cast<uint32_t>(GP4_FIELD(int32_t, obj, 0x10) >> 5)
                      - GP4_GLOBAL(uint32_t, 0x007a8dd4);
    const uint32_t dz = static_cast<uint32_t>(GP4_FIELD(int32_t, obj, 0x18) >> 5)
                      - GP4_GLOBAL(uint32_t, 0x007a8ddc);

    gp4::Regs c = {};
    c.eax = dx;
    c.edx = dz;
    gp4::call_regs(0x00401315, c);
    const uint16_t angle = static_cast<uint16_t>(c.eax);
    GP4_GLOBAL(uint16_t, 0x0088fc80) = angle;
    const uint16_t relative = static_cast<uint16_t>(heading - c.eax);
    GP4_GLOBAL(uint16_t, 0x0088fc82) = relative;

    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0088fcf0);
    const int32_t divisor = static_cast<int32_t>(mulShr14(cosQ14(relative), static_cast<int32_t>(range)) + base);
    int32_t remainder;
    GP4_GLOBAL(int32_t, 0x0088fce4) =
        gp4::idiv64(static_cast<int64_t>(static_cast<uint64_t>(base) << 14), divisor, &remainder);

    int32_t distance;
    if (static_cast<int32_t>(abs32(dx)) < static_cast<int32_t>(abs32(dz))) {
        int32_t d = cosQ14(angle);
        if (d == 0) {
            d = 1;
        }
        distance = gp4::idiv64(shl11(dz), d, &remainder);
    } else {
        int32_t d = cosQ14(static_cast<uint16_t>(0x4000u - angle));
        if (d == 0) {
            d = 1;
        }
        distance = gp4::idiv64(shl11(dx), d, &remainder);
    }

    const uint32_t clamped = distance < 0x1000 ? 0x1000u : static_cast<uint32_t>(distance);
    uint32_t scale = 0x4000000u / clamped;
    if (static_cast<int32_t>(scale) > 0x4000) {
        scale = 0x4000u;
    }
    uint32_t zoom = mulShr14(static_cast<int32_t>(scale), GP4_GLOBAL(int32_t, 0x0088fba4));

    const int32_t facing = cosQ14(static_cast<uint16_t>(relative + 0x8000u));
    uint32_t rate;
    if (facing < 0) {
        zoom = mulShr14(static_cast<int32_t>(zoom), GP4_GLOBAL(int32_t, 0x0088fce0));
        if (static_cast<int32_t>(zoom) > 0x4000) {
            zoom = 0x4000u;
        }
        rate = 0u;
    } else {
        const uint32_t along = mulShr14(distance, facing);
        const uint64_t scaled = static_cast<uint64_t>(along) * 1000u;
        uint32_t high = static_cast<uint32_t>(scaled >> 32);
        uint32_t den = range;
        if (static_cast<int32_t>(den) < 0x100) {
            den = 0x100u;
        }
        if (den <= high) {
            den = high + 1u;
            if (den == 0u) {
                den = 0xffffffffu;
                high -= 1u;
            }
        }
        uint32_t rem;
        rate = gp4::div64((static_cast<uint64_t>(high) << 32) | static_cast<uint32_t>(scaled), den, &rem);
        if (static_cast<int32_t>(rate) > 0x40000000) {
            rate = 0x40000000u;
        }
    }
    GP4_GLOBAL(uint32_t, 0x0088fce8) = zoom;
    GP4_GLOBAL(uint32_t, 0x0088fc78) = rate;

    const int32_t delta = static_cast<int16_t>(static_cast<uint16_t>(angle - GP4_GLOBAL(uint16_t, 0x00679b5c)));
    int32_t folded = delta < 0 ? -delta : delta;
    if (folded >= 0x4000) {
        folded = 0x8000 - folded;
    }
    if (delta < 0) {
        folded = -folded;
    }
    int32_t pan = static_cast<int32_t>(mulShr14(0xa0, folded)) + 0xa0;
    if (pan < 0) {
        pan = 0;
    }
    if (pan > 0x140) {
        pan = 0x140;
    }
    GP4_GLOBAL(uint32_t, 0x0088fc74) = static_cast<uint32_t>(pan) << 8;

    // Flags of the final `shl eax, 8` with eax in 0..0x140: CF = OF = AF = SF = 0, PF = 1 (low byte 0).
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::PF | (pan == 0 ? gp4::flags::ZF : 0u));
}
GP4_IMPL(0x00518f17, FUN_00518f17, "regs(ebx:ptr[0x20]:layout=obj_00518f17) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cam_00518f17")
