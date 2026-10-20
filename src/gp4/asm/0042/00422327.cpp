#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of sub left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// eax = index of a 16-byte record (x, y, z, ...) in the table behind DAT_0062a058, edx = body.
// Stores the record's offset from the origin (0x62a064/68/6c) as three doubles at body+0xa0/a8/b0,
// lets 0x00401ed3 dot them into body+0xc8, and returns that double rounded to an int (fistp), also
// left in DAT_006f2bf4. ebp keeps the last integer difference; the flags are those of its sub.
// FUNCTION: GP4 0x00422327
void __cdecl FUN_00422327(gp4::Regs* r) {
    const int32_t* record = reinterpret_cast<const int32_t*>(
        static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a058) + (r->eax << 4)));
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    const int32_t dx = record[0] - GP4_GLOBAL(int32_t, 0x0062a064);
    GP4_FIELD(double, body, 0xa0) = static_cast<double>(dx);
    const int32_t dy = record[1] - GP4_GLOBAL(int32_t, 0x0062a068);
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(dy);
    const int32_t dz = record[2] - GP4_GLOBAL(int32_t, 0x0062a06c);
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(dz);

    gp4::Regs call = *r;
    gp4::call_regs(0x00401ed3, call);

    const int32_t result = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = result;

    r->eax = static_cast<uint32_t>(result);
    r->ebp = static_cast<uint32_t>(dz);
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(static_cast<uint32_t>(record[2]), GP4_GLOBAL(uint32_t, 0x0062a06c));
}
GP4_IMPL(0x00422327, FUN_00422327, "regs(eax:u32[0..63], edx:ptr[0xd0]:f64) -> (eax, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00422397")
