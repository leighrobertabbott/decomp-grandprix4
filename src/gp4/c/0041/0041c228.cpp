#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit sub / cmp left, right.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The four 0x300-byte wheel records at 0x006380d8: only the fields used here.
struct Unknown_006380d8 {
    uint8_t Unknown00[0x1c];
    int32_t Unknown1C;
    uint8_t Unknown20[0x40 - 0x20];
    int32_t Unknown40;
    uint8_t Unknown44[0x48 - 0x44];
    int32_t Unknown48;
    uint8_t Unknown4C[0x1a0 - 0x4c];
    int32_t Unknown1A0;
    uint8_t Unknown1A4[0x1cc - 0x1a4];
    int32_t Unknown1CC;
    uint8_t Unknown1D0[0x258 - 0x1d0];
    int32_t Unknown258;
    uint8_t Unknown25C[0x260 - 0x25c];
    int32_t Unknown260;
    int32_t Unknown264;
    uint8_t Unknown268[0x300 - 0x268];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// imul by gain; shrd eax, edx, 14: low dword of the 64-bit product >> 14. *high is edx.
static inline uint32_t mulShr14(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0041c228
// Steering/axle update of the wheel records at 0x006380d8: the signed difference of the two
// front/rear pair sums (+0x1c + +0x40 against record 1) is scaled through a curve table
// (0x00411891, table pointer [0x62f250 + [0x62f24c]*4]) and the constants 0x6f29f4, 0x6013c0,
// 0x601468, 0x6013c8, rounded, given the sign of the difference and scaled by [0x62f238] (Q14) into
// +0x4cc (and its negation into +0x1cc). After the wheel-pair update 0x0041c145 the records 2 and 3
// get their differences (+0x48 - +0x1a0 -> +0x260) and Q14 scaled values (+0x258).
void __cdecl FUN_0041c228(gp4::Regs* r) {
    Unknown_006380d8* wheels = reinterpret_cast<Unknown_006380d8*>(static_cast<uintptr_t>(0x006380d8u));

    const uint32_t front = static_cast<uint32_t>(wheels[0].Unknown1C) + static_cast<uint32_t>(wheels[0].Unknown40);
    const uint32_t rear = static_cast<uint32_t>(wheels[1].Unknown1C) + static_cast<uint32_t>(wheels[1].Unknown40);
    const int32_t difference = static_cast<int32_t>(front - rear);
    int32_t magnitude = difference;
    if (difference < 0) {
        magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(difference));
    }
    GP4_GLOBAL(int32_t, 0x006f2bf4) = magnitude;

    gp4::x87::Ext value = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    value = value * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29f4));
    value = value * gp4::x87::ext(GP4_GLOBAL(double, 0x006013c0));

    const uint32_t tableIndex = GP4_GLOBAL(uint32_t, 0x0062f24c);
    gp4::Regs curve = *r;
    curve.edx = GP4_ARRAY(uint32_t, 0x0062f250)[tableIndex];
    curve.st_in = 1;
    curve.st_out = 1;
    gp4::x87::set_in_st(curve, 0, value);
    gp4::call_regs(0x00411891, curve);
    value = gp4::x87::out_st(curve, 0);

    value = value * gp4::x87::ext(GP4_GLOBAL(double, 0x00601468));
    value = value * gp4::x87::ext(GP4_GLOBAL(double, 0x006013c8));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(value);

    int32_t scaled = GP4_GLOBAL(int32_t, 0x006f2bf4);
    if (difference < 0) {
        scaled = static_cast<int32_t>(0u - static_cast<uint32_t>(scaled));
    }
    uint32_t high;
    const uint32_t steer = mulShr14(scaled, GP4_GLOBAL(int32_t, 0x0062f238), &high);
    wheels[1].Unknown1CC = static_cast<int32_t>(steer);
    wheels[0].Unknown1CC = static_cast<int32_t>(0u - steer);

    gp4::Regs update = *r;
    update.st_in = 0;
    update.st_out = 0;
    update.ebx = 0x006380d8u;
    update.eax = 0u - steer;
    update.edx = high;
    gp4::call_regs(0x0041c145, update);

    Unknown_006380d8* records = reinterpret_cast<Unknown_006380d8*>(static_cast<uintptr_t>(update.ebx));

    const uint32_t rearDifference = static_cast<uint32_t>(records[2].Unknown48) - static_cast<uint32_t>(records[2].Unknown1A0);
    records[2].Unknown260 = static_cast<int32_t>(rearDifference);
    uint32_t edx;
    uint32_t result = mulShr14(static_cast<int32_t>(rearDifference), GP4_GLOBAL(int32_t, 0x00601a88), &edx);
    records[2].Unknown258 = static_cast<int32_t>(result - static_cast<uint32_t>(records[2].Unknown264));

    const uint32_t lastDifference = static_cast<uint32_t>(records[3].Unknown48) - static_cast<uint32_t>(records[3].Unknown1A0);
    records[3].Unknown260 = static_cast<int32_t>(lastDifference);
    result = mulShr14(static_cast<int32_t>(lastDifference), GP4_GLOBAL(int32_t, 0x00601a8c), &edx);
    const uint32_t subtrahend = static_cast<uint32_t>(records[3].Unknown264);
    records[3].Unknown258 = static_cast<int32_t>(result - subtrahend);

    r->eax = result - subtrahend;
    r->ebx = update.ebx;
    r->ecx = update.ecx;
    r->edx = edx;
    r->esi = update.esi;
    r->edi = update.edi;
    r->ebp = update.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(result, subtrahend);
}
GP4_IMPL(0x0041c228, FUN_0041c228, "regs(esi:ptr[0x6a]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=curves_0062ecb0+dw_0041c027+state_0041bce9+state_0041b950+state_0041bd56+state_0041bbef+state_0041bd28+div_0041bc3a+state_0041befb+state_0041c145+state_0041c228")
