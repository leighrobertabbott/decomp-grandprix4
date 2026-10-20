#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit ADD (the 0x8d5 arithmetic flags).
static inline uint32_t add32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((~(left ^ right)) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00420d3f_Car {
    uint8_t Unknown00[0x1a8];
    uint32_t Packed1A8;
    uint8_t Unknown1AC[0x18];
    uint8_t Count1C4;
};

// The 0x300-byte records at 0x6380d8: add `value` to the dword at +0x38 of record `index`.
static __declspec(noinline) uint32_t addToRecord(uint8_t* records, uint32_t index, uint32_t value) {
    uint32_t* slot = reinterpret_cast<uint32_t*>(records + index * 0x300u + 0x38u);
    const uint32_t before = *slot;
    *slot = before + value;
    return add32Flags(before, value);
}

// Adds the running total of 0x00637f78 to 0x006228a4.
static __declspec(noinline) uint32_t addFrameTime(uint32_t* eaxOut) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00637f78);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x006228a4);
    GP4_GLOBAL(uint32_t, 0x006228a4) = before + value;
    *eaxOut = value;
    return add32Flags(before, value);
}

// Per-frame car update, the longer sibling of 0x00420c87: 0x41d5a1, 0x42045e, 0x41f805 and
// 0x41f613 on the car at esi; when the packed four-byte impulse at +0x1a8 is nonzero its count
// byte +0x1c4 is decremented (the impulse clears at zero) and each byte, scaled by
// <<13 * [0x629e60] >> 14, is added to dword +0x38 of the four 0x300-byte records at 0x6380d8;
// then 0x4208f3 and 0x4205a0, [0x6228a4] += [0x637f78], and 0x420ed6, 0x4200a7, 0x42027f.
// Registers and flags flow through the callees, so the result is the state 0x42027f leaves.
// FUNCTION: GP4 0x00420d3f
void __cdecl FUN_00420d3f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0;
    gp4::call_regs(0x0041d5a1, c);
    gp4::call_regs(0x0042045e, c);
    gp4::call_regs(0x0041f805, c);
    gp4::call_regs(0x0041f613, c);

    Unknown_00420d3f_Car* car = reinterpret_cast<Unknown_00420d3f_Car*>(static_cast<uintptr_t>(c.esi));
    if (car->Packed1A8 == 0) {
        c.eflags = (c.eflags & ~0x8d5u) | 0x44u;
    } else {
        c.ebp = car->Packed1A8;
        car->Count1C4 = static_cast<uint8_t>(car->Count1C4 - 1u);
        if (car->Count1C4 == 0) {
            car->Packed1A8 = 0;
        }
        c.ebx = 0x006380d8u;
        const int32_t factor = GP4_GLOBAL(int32_t, 0x00629e60);
        uint32_t flags = 0;
        for (uint32_t index = 0; index < 4; index++) {
            c.eax = c.ebp;
            c.ebp >>= 8;
            c.eax = (c.eax & 0xffu) << 13;
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(c.eax)) * factor;
            c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            flags = addToRecord(GP4_ARRAY(uint8_t, 0x006380d8), index, c.eax);
        }
        c.eflags = (c.eflags & ~0x8d5u) | flags;
    }

    gp4::call_regs(0x004208f3, c);
    gp4::call_regs(0x004205a0, c);

    uint32_t eax = 0;
    const uint32_t flags = addFrameTime(&eax);
    c.eax = eax;
    c.eflags = (c.eflags & ~0x8d5u) | flags;

    gp4::call_regs(0x00420ed6, c);
    gp4::call_regs(0x004200a7, c);
    gp4::call_regs(0x0042027f, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00420d3f, FUN_00420d3f, "regs(esi:ptr[0x420]:layout=car_00420d3f, ecx:u32[0..8], ebp:u32[0..8], edx:u32[0..8]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); cases=300; globals=fuzz:i32+layout=view_00609860+recs_006380d8+state_004178e0+integ_004200a7+state_0041f489+spin_0041f28d+wheel_004205a0+lim_00420e6b+state_0041f613+state_004209ae+state_0042027f+state_00420e1d+wheels_00420d3f")
