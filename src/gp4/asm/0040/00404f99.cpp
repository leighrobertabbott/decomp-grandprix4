#include <gp4/gp4.h>

// "imul" (signed 32x32 -> edx:eax) followed by "shrd eax, edx, count": bits count..count+31
// of the product; *high receives edx (the product's high dword).
static inline uint32_t mulShr(int32_t a, int32_t b, unsigned count, uint32_t* high) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    *high = static_cast<uint32_t>(product >> 32);
    return static_cast<uint32_t>(product >> count);
}

static inline int32_t clampNonNegative(int32_t value) {
    return value < 0 ? 0 : value;
}

// FUNCTION: GP4 0x00404f99
// Drivetrain step for the car record in esi: eax/edx are stored to 0x006030b4/0x006030bc, the
// helpers 0x404aa3/0x404ed0/0x404f33 refresh their state, four clamped Q14 terms are written to
// 0x0060309c..0x006030a8 and mapped through 0x411c26 into 0x006030f4..0x00603100, 0x404bcf
// rescales them, and their combined, gain-scaled sum goes through 0x404e9c into 0x00602d70.
// ebx/ecx/edi/ebp are preserved; eax, edx and the flags are those 0x404e9c leaves.
void __cdecl FUN_00404f99(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t high;

    GP4_GLOBAL(uint32_t, 0x006030bc) = r->edx;
    GP4_GLOBAL(uint32_t, 0x006030b4) = r->eax;
    gp4::call_regs(0x00404aa3, c);

    const int32_t speed = GP4_GLOBAL(int32_t, 0x006030b4);
    const uint32_t square = mulShr(speed, speed, 0x17, &high);
    GP4_GLOBAL(uint32_t, 0x006030cc) = square;
    c.eax = square;
    c.edx = high;
    c.ebp = square;
    gp4::call_regs(0x00404ed0, c);

    const uint32_t scaledLoad = mulShr(GP4_GLOBAL(int32_t, 0x006030bc), GP4_FIELD(uint16_t, car, 0xb2), 0x10, &high);
    const uint32_t load = mulShr(static_cast<int32_t>(scaledLoad), GP4_GLOBAL(int32_t, 0x006030cc), 0xe, &high);
    c.ebx = load;
    const int32_t factor = GP4_GLOBAL(int32_t, 0x006030b8);
    if (factor != 0) {
        GP4_GLOBAL(int32_t, 0x006030c0) =
            static_cast<int32_t>(mulShr(factor, static_cast<int32_t>(load), 0xe, &high)) >> 1;
        const int32_t base = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(uint16_t, car, 0xb2)) << 8);
        const uint32_t offset = mulShr(base, GP4_GLOBAL(int32_t, 0x006030b8), 0xe, &high);
        GP4_GLOBAL(uint32_t, 0x006030c4) = offset;
        c.eax = offset;
    } else {
        GP4_GLOBAL(uint32_t, 0x006030c0) = 0;
        GP4_GLOBAL(uint32_t, 0x006030c4) = 0;
        c.eax = 0;
    }
    c.edx = high;
    gp4::call_regs(0x00404f33, c);

    GP4_GLOBAL(uint32_t, 0x006030b0) =
        mulShr(static_cast<int32_t>(load - GP4_GLOBAL(uint32_t, 0x006030c4)), GP4_GLOBAL(int32_t, 0x006013b4), 0xe, &high);
    const uint32_t forceSum = GP4_GLOBAL(uint32_t, 0x00603104) + GP4_GLOBAL(uint32_t, 0x00603108)
        + GP4_GLOBAL(uint32_t, 0x0060310c) + GP4_GLOBAL(uint32_t, 0x00603110);
    const uint32_t force = mulShr(static_cast<int32_t>(forceSum), GP4_GLOBAL(int32_t, 0x006013b8), 0xe, &high);
    GP4_GLOBAL(uint32_t, 0x006030ac) = force;
    const int32_t halfForce = static_cast<int32_t>(force) >> 1;
    c.ebx = static_cast<uint32_t>(halfForce);

    const int32_t grip = static_cast<int32_t>(
        mulShr(GP4_FIELD(uint16_t, car, 0xb8), GP4_GLOBAL(int32_t, 0x0060312c), 0xe, &high)) >> 1;
    GP4_GLOBAL(int32_t, 0x0060308c) = grip;
    GP4_GLOBAL(int32_t, 0x00603094) = grip;

    int32_t front = static_cast<int32_t>(mulShr(GP4_GLOBAL(int32_t, 0x0060308c), static_cast<int32_t>(square), 0xe, &high))
        + (GP4_GLOBAL(int32_t, 0x006030c8) >> 1);
    front = clampNonNegative(front);
    GP4_GLOBAL(int32_t, 0x00603090) = front + GP4_GLOBAL(int32_t, 0x006030c0);

    int32_t rear = static_cast<int32_t>(mulShr(GP4_GLOBAL(int32_t, 0x00603094), static_cast<int32_t>(square), 0xe, &high))
        + (GP4_GLOBAL(int32_t, 0x006030c8) >> 1);
    rear = clampNonNegative(rear);
    GP4_GLOBAL(int32_t, 0x00603098) = rear + GP4_GLOBAL(int32_t, 0x006030c0);

    // Left/right split by 0x0062ec18 (Q14) and front/rear weights 0x0062ec10 / 0x0062ec14.
    const int32_t inner = static_cast<int32_t>(
        mulShr(0x4000 - GP4_GLOBAL(int32_t, 0x0062ec18), GP4_GLOBAL(int32_t, 0x006030b0), 0xe, &high));

    int32_t term = clampNonNegative(static_cast<int32_t>(
        mulShr(GP4_GLOBAL(int32_t, 0x00603090), GP4_GLOBAL(int32_t, 0x0062ec10), 0xe, &high)) + inner + halfForce);
    GP4_GLOBAL(int32_t, 0x0060309c) = term;
    c.eax = static_cast<uint32_t>(term);
    c.ecx = 0;
    c.edx = high;
    c.edi = static_cast<uint32_t>(inner);
    gp4::call_regs(0x00411c26, c);
    GP4_GLOBAL(uint32_t, 0x006030f4) = c.eax;

    term = clampNonNegative(static_cast<int32_t>(
        mulShr(GP4_GLOBAL(int32_t, 0x00603098), GP4_GLOBAL(int32_t, 0x0062ec10), 0xe, &high)) - inner + halfForce);
    GP4_GLOBAL(int32_t, 0x006030a0) = term;
    c.eax = static_cast<uint32_t>(term);
    c.ecx = 0;
    c.edx = high;
    gp4::call_regs(0x00411c26, c);
    GP4_GLOBAL(uint32_t, 0x006030f8) = c.eax;

    const int32_t outer = static_cast<int32_t>(
        mulShr(GP4_GLOBAL(int32_t, 0x0062ec18), GP4_GLOBAL(int32_t, 0x006030b0), 0xe, &high));

    term = clampNonNegative(static_cast<int32_t>(
        mulShr(GP4_GLOBAL(int32_t, 0x00603090), GP4_GLOBAL(int32_t, 0x0062ec14), 0xe, &high)) + outer - halfForce);
    GP4_GLOBAL(int32_t, 0x006030a4) = term;
    c.eax = static_cast<uint32_t>(term);
    c.ecx = 2;
    c.edx = high;
    c.edi = static_cast<uint32_t>(outer);
    gp4::call_regs(0x00411c26, c);
    GP4_GLOBAL(uint32_t, 0x006030fc) = c.eax;

    term = clampNonNegative(static_cast<int32_t>(
        mulShr(GP4_GLOBAL(int32_t, 0x00603098), GP4_GLOBAL(int32_t, 0x0062ec14), 0xe, &high)) - outer - halfForce);
    GP4_GLOBAL(int32_t, 0x006030a8) = term;
    c.eax = static_cast<uint32_t>(term);
    c.ecx = 2;
    c.edx = high;
    gp4::call_regs(0x00411c26, c);
    GP4_GLOBAL(uint32_t, 0x00603100) = c.eax;

    gp4::cdecl_<0x00404bcf, void>();

    const uint32_t total = GP4_GLOBAL(uint32_t, 0x006030f4) + GP4_GLOBAL(uint32_t, 0x006030f8)
        + GP4_GLOBAL(uint32_t, 0x006030fc) + GP4_GLOBAL(uint32_t, 0x00603100);
    uint32_t drive = mulShr(static_cast<int32_t>(total), GP4_FIELD(uint16_t, car, 0xa2), 0xe, &high);
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 1u) == 0u) {
        const uint32_t loss = mulShr(GP4_FIELD(uint16_t, car, 0xba), GP4_GLOBAL(int32_t, 0x0062d298), 0xe, &high);
        drive = mulShr(static_cast<int32_t>(0x4000u - loss), static_cast<int32_t>(drive), 0xe, &high);
    }
    drive = mulShr(static_cast<int32_t>(drive), GP4_FIELD(uint16_t, car, 0x10e), 0xe, &high);
    drive = mulShr(static_cast<int32_t>(drive), GP4_GLOBAL(int32_t, 0x00602d6c), 0xe, &high);
    drive = mulShr(static_cast<int32_t>(drive), GP4_GLOBAL(int32_t, 0x00602d68), 0xe, &high);
    c.eax = drive + GP4_GLOBAL(uint32_t, 0x006030c4);
    c.edx = high;
    gp4::call_regs(0x00404e9c, c);
    GP4_GLOBAL(uint32_t, 0x00602d70) = c.eax;

    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00404f99, FUN_00404f99, "regs(eax:u32, edx:u32, esi:ptr[0x118]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00404ed0+state_00404f33+engine_00413606+drive_004054e6")
