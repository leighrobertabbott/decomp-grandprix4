#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit OR result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Runs the per-car step 0x00420d3f [0x006228b4] times (at least once) after 0x00420880, copying the vector
// 0x00638034/30/38 to 0x00622e98.. and building the orientation sines/cosines (0x00401d5c/80/a2) before every step;
// then 0x004209ae over the state 0x006380d8 and the mean of the per-step sums: [0x006228a4] / steps -> 0x00623e9c.
// Marks bit 6 of the car byte +0x21c (esi). All registers flow from callee to callee.
// FUNCTION: GP4 0x00420bf9
void __cdecl FUN_00420bf9(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00420880, c);
    GP4_GLOBAL(int32_t, 0x006228a0) = 0;
    GP4_GLOBAL(int32_t, 0x006228a4) = 0;
    do {
        c.edx = 0x00622e98u;
        GP4_GLOBAL(uint32_t, 0x00622e98) = GP4_GLOBAL(uint32_t, 0x00638034);
        GP4_GLOBAL(uint32_t, 0x00622e9c) = GP4_GLOBAL(uint32_t, 0x00638030);
        GP4_GLOBAL(uint32_t, 0x00622ea0) = GP4_GLOBAL(uint32_t, 0x00638038);
        gp4::call_regs(0x00401d5c, c);
        gp4::call_regs(0x00401d80, c);
        gp4::call_regs(0x00401da2, c);
        gp4::call_regs(0x00420d3f, c);
        GP4_GLOBAL(int32_t, 0x006228a0) = GP4_GLOBAL(int32_t, 0x006228a0) + 1;
    } while (GP4_GLOBAL(int32_t, 0x006228a0) < GP4_GLOBAL(int32_t, 0x006228b4));

    c.ebx = 0x006380d8u;
    gp4::call_regs(0x004209ae, c);

    int32_t steps = GP4_GLOBAL(int32_t, 0x006228a0);
    if (steps == 0) {
        steps = 1;
    }
    int32_t remainder;
    const int32_t sum = GP4_GLOBAL(int32_t, 0x006228a4);
    const int32_t mean = gp4::idiv64(static_cast<int64_t>(sum), steps, &remainder);
    GP4_GLOBAL(int32_t, 0x00623e9c) = mean;

    const uint32_t flags21c = GP4_FIELD(uint8_t, car, 0x21c) | 0x40u;
    GP4_FIELD(uint8_t, car, 0x21c) = static_cast<uint8_t>(flags21c);

    c.eax = static_cast<uint32_t>(mean);
    c.edx = static_cast<uint32_t>(remainder);
    c.ebp = static_cast<uint32_t>(steps);
    c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(flags21c);
    *r = c;
}
GP4_IMPL(0x00420bf9, FUN_00420bf9, "regs(esi:ptr[0x420]:layout=car_00420d3f, ecx:u32, ebp:u32, ebx:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+recs_006380d8+state_004178e0+integ_004200a7+state_0041f489+spin_0041f28d+wheel_004205a0+lim_00420e6b+state_0041f613+state_004209ae+state_0042027f+state_00420e1d+wheels_00420d3f+lim_00420bf9")
