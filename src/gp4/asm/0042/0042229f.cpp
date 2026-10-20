#include <gp4/gp4.h>

// Flags of the 32-bit "sub left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Takes record number eax of the 16-byte records at [0x0062a058], stores its three dwords minus the
// point [0x0062a064]/[0x0062a068]/[0x0062a06c] as doubles at 0x00622c80/88/90 (each first through the
// dword 0x006f2bf4), converts that vector to angles with 0x00402305, copies the two angles and the
// zero to +0x10/+0x18/+0x20 of the block 0x00622f68 and has 0x00401d80 / 0x00401da2 build the
// sin/cos pairs and the rotation matrix there. eax = the last difference, edx = 0x00622f68, the flags
// are those of the last `sub`; the three callees leave every register and flag alone.
// FUNCTION: GP4 0x0042229f
void __cdecl FUN_0042229f(gp4::Regs* r) {
    const uint32_t* record = reinterpret_cast<const uint32_t*>(GP4_GLOBAL(uint32_t, 0x0062a058) + (r->eax << 4));

    uint32_t difference = record[0] - GP4_GLOBAL(uint32_t, 0x0062a064);
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = difference;
    GP4_GLOBAL(double, 0x00622c80) = static_cast<double>(static_cast<int32_t>(difference));

    difference = record[1] - GP4_GLOBAL(uint32_t, 0x0062a068);
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = difference;
    GP4_GLOBAL(double, 0x00622c88) = static_cast<double>(static_cast<int32_t>(difference));

    const uint32_t lastLeft = record[2];
    const uint32_t lastRight = GP4_GLOBAL(uint32_t, 0x0062a06c);
    difference = lastLeft - lastRight;
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = difference;
    GP4_GLOBAL(double, 0x00622c90) = static_cast<double>(static_cast<int32_t>(difference));

    gp4::cdecl_<0x00402305, void>();

    uint8_t* block = GP4_ARRAY(uint8_t, 0x00622f68);
    GP4_FIELD(double, block, 0x10) = GP4_GLOBAL(double, 0x00622c98);
    GP4_FIELD(double, block, 0x18) = GP4_GLOBAL(double, 0x00622ca0);
    GP4_FIELD(double, block, 0x20) = GP4_GLOBAL(double, 0x00622ca8);

    gp4::Regs c = *r;
    c.edx = 0x00622f68u;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00401d80, c);
    gp4::call_regs(0x00401da2, c);

    r->eax = difference;
    r->edx = 0x00622f68u;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(lastLeft, lastRight);
}
GP4_IMPL(0x0042229f, FUN_0042229f, "regs(eax:u32[0..15]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=list_0062a048")
