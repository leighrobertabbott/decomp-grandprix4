#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((~(left ^ right) & (left ^ result)) >> 20) & 0x800u);
}

// imul (one operand) then shrd eax, edx, n: the low dword of the 64-bit product shifted right.
static inline int32_t mulShift(int32_t a, int32_t b, int n) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> n));
}

static inline int64_t mul64(int32_t a, int32_t b) {
    return static_cast<int64_t>(a) * static_cast<int64_t>(b);
}

static inline int64_t shl64(int64_t value, int n) {
    return static_cast<int64_t>(static_cast<uint64_t>(value) << n);
}

static inline int32_t times3(int32_t value) {
    return static_cast<int32_t>(static_cast<uint32_t>(value) * 3u);
}

static inline int32_t quotient(int64_t dividend, int32_t divisor) {
    int32_t remainder;
    return gp4::idiv64(dividend, divisor, &remainder);
}

static inline int32_t& dw(uint8_t* base, uint32_t off) {
    return *reinterpret_cast<int32_t*>(base + off);
}

static inline void addTo(uint8_t* base, uint32_t off, uint32_t delta) {
    *reinterpret_cast<uint32_t*>(base + off) += delta;
}

static inline void store64(uint32_t lowAddr, int64_t value) {
    GP4_GLOBAL(uint32_t, lowAddr) = static_cast<uint32_t>(static_cast<uint64_t>(value));
    GP4_GLOBAL(uint32_t, lowAddr + 4u) = static_cast<uint32_t>(static_cast<uint64_t>(value) >> 32);
}

static inline int64_t load64(uint32_t lowAddr) {
    return static_cast<int64_t>((static_cast<uint64_t>(GP4_GLOBAL(uint32_t, lowAddr + 4u)) << 32) | GP4_GLOBAL(uint32_t, lowAddr));
}

// Load of one 0x300-byte record (four Q10 products and the Q11 offset term).
static int32_t recordLoad(uint8_t* rec) {
    uint32_t sum = static_cast<uint32_t>(mulShift(dw(rec, 0x144), dw(rec, 0x14c), 10));
    sum += static_cast<uint32_t>(mulShift(dw(rec, 0x148), dw(rec, 0x150), 10));
    sum += static_cast<uint32_t>(mulShift(dw(rec, 0x124), dw(rec, 0x12c), 10));
    sum += static_cast<uint32_t>(mulShift(dw(rec, 0x128), dw(rec, 0x130), 10));
    const int32_t offset = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00637f9c) - static_cast<uint32_t>(dw(rec, 0xcc)));
    sum += static_cast<uint32_t>(mulShift(offset, dw(rec, 0x9c), 11));
    return static_cast<int32_t>(sum);
}

// Predictor terms +0x108 / +0x10c of a record from position x, velocity v and the value a at
// car + aOff (stored doubled).
static void predict(uint8_t* rec, int32_t x, int32_t v, uint8_t* car, uint32_t aOff) {
    const uint32_t base = (static_cast<uint32_t>(v) << 1) + static_cast<uint32_t>(mulShift(times3(x), GP4_GLOBAL(int32_t, 0x00602bc4), 14));
    const uint32_t doubled = static_cast<uint32_t>(dw(car, aOff)) << 1;
    dw(rec, 0x108) = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(doubled) >> 2) + base);
    dw(rec, 0x10c) = static_cast<int32_t>(((base + static_cast<uint32_t>(v)) << 1) + doubled);
}

// Right-hand side of a record: (+0x108 * q >> 6) * +0xf4 + (load * q << 4) + (+0x9c << 5) * +0x10c.
static int64_t rightHandSide(uint8_t* rec, int32_t load) {
    const int32_t q = GP4_GLOBAL(int32_t, 0x00602bc0);
    int64_t sum = mul64(mulShift(dw(rec, 0x108), q, 6), dw(rec, 0xf4));
    sum += shl64(mul64(load, q), 4);
    sum += mul64(static_cast<int32_t>(static_cast<uint32_t>(dw(rec, 0x9c)) << 5), dw(rec, 0x10c));
    return sum;
}

// New velocity (stored at velocityAddr) and the value written to car + aOff for one record of the
// pair, given its load and the coupled position of the other record.
static void velocityAndRate(uint8_t* rec, int32_t load, int32_t otherPosition, int32_t coupling,
                            uint32_t velocityAddr, uint8_t* car, uint32_t aOff) {
    int64_t shared = shl64(static_cast<int64_t>(times3(load)), 20);
    shared += shl64(mul64(times3(otherPosition), coupling), 10);

    int64_t velocity = shared;
    velocity += mul64(mulShift(times3(dw(rec, 0x9c)), GP4_GLOBAL(int32_t, 0x00602bc4), 9), dw(rec, 0x10c));
    velocity += mul64(static_cast<int32_t>(0u - static_cast<uint32_t>(dw(rec, 0x100))), dw(rec, 0x108));
    GP4_GLOBAL(int32_t, velocityAddr) = quotient(velocity, dw(rec, 0x104));

    int64_t rate = shl64(shared, 1);
    rate += mul64(static_cast<int32_t>(static_cast<uint32_t>(times3(static_cast<int32_t>(static_cast<uint32_t>(dw(rec, 0xf4)) << 10))) << 1),
                  dw(rec, 0x108));
    rate += mul64(static_cast<int32_t>(0u - static_cast<uint32_t>(dw(rec, 0xf8))), dw(rec, 0x10c));
    dw(car, aOff) = quotient(rate, dw(rec, 0x104));
}

// One axle: records first / first + 0x300 coupled through dword +0x110 of the second record; car
// positions at car + posOff (+4), velocities at car + velOff (+4), rates at car + rateOff (+4).
static __declspec(noinline) void solvePair(uint8_t* first, uint8_t* car, uint32_t posOff, uint32_t velOff, uint32_t rateOff) {
    uint8_t* second = first + 0x300;

    GP4_GLOBAL(int32_t, 0x00602b84) = dw(car, posOff);
    GP4_GLOBAL(int32_t, 0x00602b80) = dw(car, posOff);
    GP4_GLOBAL(int32_t, 0x00602b8c) = dw(car, posOff + 4u);
    GP4_GLOBAL(int32_t, 0x00602b88) = dw(car, posOff + 4u);
    GP4_GLOBAL(int32_t, 0x00602b90) = dw(car, velOff);
    GP4_GLOBAL(int32_t, 0x00602b98) = dw(car, velOff);
    GP4_GLOBAL(int32_t, 0x00602b94) = dw(car, velOff + 4u);
    GP4_GLOBAL(int32_t, 0x00602b9c) = dw(car, velOff + 4u);

    GP4_GLOBAL(int32_t, 0x00602ba8) = recordLoad(first);
    GP4_GLOBAL(int32_t, 0x00602bac) = recordLoad(second);

    predict(first, GP4_GLOBAL(int32_t, 0x00602b80), GP4_GLOBAL(int32_t, 0x00602b90), car, rateOff);
    predict(second, GP4_GLOBAL(int32_t, 0x00602b88), GP4_GLOBAL(int32_t, 0x00602b94), car, rateOff + 4u);

    store64(0x00602bb0, rightHandSide(first, GP4_GLOBAL(int32_t, 0x00602ba8)));
    store64(0x00602bb8, rightHandSide(second, GP4_GLOBAL(int32_t, 0x00602bac)));

    // Solve the coupled 2x2 system for the new positions.
    const int32_t q = GP4_GLOBAL(int32_t, 0x00602bc0);
    const int32_t coupling = mulShift(dw(second, 0x110), q, 6);
    const int32_t partSecond = quotient(load64(0x00602bb8), dw(second, 0x104));
    const int32_t pivotFirst = static_cast<int32_t>(static_cast<uint32_t>(dw(first, 0x104))
        - static_cast<uint32_t>(quotient(mul64(coupling, coupling), dw(second, 0x104))));
    GP4_GLOBAL(int32_t, 0x00602b80) = quotient(mul64(coupling, partSecond) + load64(0x00602bb0), pivotFirst);
    const int32_t partFirst = quotient(load64(0x00602bb0), dw(first, 0x104));
    const int32_t pivotSecond = static_cast<int32_t>(static_cast<uint32_t>(dw(second, 0x104))
        - static_cast<uint32_t>(quotient(mul64(coupling, coupling), dw(first, 0x104))));
    GP4_GLOBAL(int32_t, 0x00602b88) = quotient(mul64(coupling, partFirst) + load64(0x00602bb8), pivotSecond);

    velocityAndRate(first, GP4_GLOBAL(int32_t, 0x00602ba8), GP4_GLOBAL(int32_t, 0x00602b88), dw(second, 0x110),
                    0x00602b90, car, rateOff);
    velocityAndRate(second, GP4_GLOBAL(int32_t, 0x00602bac), GP4_GLOBAL(int32_t, 0x00602b80), dw(second, 0x110),
                    0x00602b94, car, rateOff + 4u);

    addTo(car, posOff, GP4_GLOBAL(uint32_t, 0x00602b80) - GP4_GLOBAL(uint32_t, 0x00602b84));
    addTo(car, posOff + 4u, GP4_GLOBAL(uint32_t, 0x00602b88) - GP4_GLOBAL(uint32_t, 0x00602b8c));
    addTo(car, velOff, GP4_GLOBAL(uint32_t, 0x00602b90) - GP4_GLOBAL(uint32_t, 0x00602b98));
}

// FUNCTION: GP4 0x00420ed6
// Implicit step of the four 0x300-byte records at 0x006380d8 as two coupled pairs (records 0/1 with
// the car block's dwords +0x32c/+0x34c/+0x30c, records 2/3 with +0x334/+0x354/+0x314). q =
// [0x006228ac] (kept at 0x602bc0) and 2^30 / q (0x602bc4) scale the step; 0x0042181c and 0x004217a8
// prepare the record sums and divisors first. pushal/popal keep every register; the flags are those
// of the final `add [esi+0x358], eax`.
void __cdecl FUN_00420ed6(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x00602bc0) = GP4_GLOBAL(uint32_t, 0x006228ac);
    uint32_t remainder;
    GP4_GLOBAL(uint32_t, 0x00602bc4) = gp4::div64(0x40000000ull, GP4_GLOBAL(uint32_t, 0x00602bc0), &remainder);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0042181c, c);
    gp4::call_regs(0x004217a8, c);

    uint8_t* records = GP4_ARRAY(uint8_t, 0x006380d8);
    solvePair(records, car, 0x32cu, 0x34cu, 0x30cu);
    addTo(car, 0x350, GP4_GLOBAL(uint32_t, 0x00602b94) - GP4_GLOBAL(uint32_t, 0x00602b9c));

    solvePair(records + 0x600, car, 0x334u, 0x354u, 0x314u);
    const uint32_t old = static_cast<uint32_t>(dw(car, 0x358));
    const uint32_t delta = GP4_GLOBAL(uint32_t, 0x00602b94) - GP4_GLOBAL(uint32_t, 0x00602b9c);
    dw(car, 0x358) = static_cast<int32_t>(old + delta);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(old, delta);
}
GP4_IMPL(0x00420ed6, FUN_00420ed6, "regs(esi:ptr[0x420]:layout=car_00420d3f) -> (cf, pf, af, zf, sf, of); globals=layout=wheels_00420d3f")
