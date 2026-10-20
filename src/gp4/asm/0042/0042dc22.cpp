#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

struct Unknown_0042dc22 {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
};

// Scans the table of bodies at 0x006078e4 from index [0x006078dc] up to [0x006078e0]: for every
// body whose word at +0x68 is <= (esi->+0x68 + [0x00603afc]) it calls 0x0042db77 (ebx = body)
// and stops with CF clear when that routine returns a no-carry result whose |ax| is below
// [0x00603ae8]. CF set means no body qualified. ebx is left holding the last body examined;
// eax holds the last result of 0x0042db77; ebp/ecx/edx are preserved.
// FUNCTION: GP4 0x0042dc22
void __cdecl FUN_0042dc22(gp4::Regs* r) {
    const Unknown_0042dc22* subject = reinterpret_cast<const Unknown_0042dc22*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEdx = r->edx;
    const int16_t target = static_cast<int16_t>(static_cast<uint16_t>(subject->Unknown68) + GP4_GLOBAL(uint16_t, 0x00603afc));
    uint32_t index = GP4_GLOBAL(uint32_t, 0x006078dc);

    if (index < GP4_GLOBAL(uint32_t, 0x006078e0)) {
        do {
            const uint32_t entry = GP4_ARRAY(uint32_t, 0x006078e4)[index];
            const Unknown_0042dc22* other = reinterpret_cast<const Unknown_0042dc22*>(static_cast<uintptr_t>(entry));
            r->ebx = entry;
            if (target >= other->Unknown68) {
                r->edx = index;
                gp4::call_regs(0x0042db77, *r);
                r->edx = index;
                if ((r->eflags & 1u) == 0u) {
                    uint16_t ax = static_cast<uint16_t>(r->eax);
                    if (static_cast<int16_t>(ax) < 0) {
                        ax = static_cast<uint16_t>(0u - ax);
                    }
                    r->eax = (r->eax & 0xffff0000u) | ax;
                    const uint16_t limit = GP4_GLOBAL(uint16_t, 0x00603ae8);
                    if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
                        // "cmp ax, [0x603ae8]" then clc
                        r->edx = savedEdx;
                        r->eflags = (r->eflags & ~0x8d5u) | (subFlags16(ax, limit) & ~1u);
                        return;
                    }
                }
            }
            index = index + 1u;
        } while (index < GP4_GLOBAL(uint32_t, 0x006078e0));
    }

    // "cmp edx, [0x6078e0]" then stc
    r->edx = savedEdx;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(index, GP4_GLOBAL(uint32_t, 0x006078e0)) | 1u;
}
GP4_IMPL(0x0042dc22, FUN_0042dc22, "regs(eax:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
