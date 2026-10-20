#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00424814
// pushal ... popal routine over the 22 car records at 0x0066fef0 (stride 0x41c). For every car
// whose dwords +0x40 and +0x44 differ (the parts mask changes), the difference mask is split into
// connected groups by 0x004447b2 (ebp = remaining mask, eax = next group, CF = none) and 0x00424d65
// queues each group as debris (the group is kept in 0x00639634, the car's dword +0x48 in
// 0x00639638), for at most 0x17 groups per car or until the mask is empty or a callee reports
// failure with CF. Registers are restored by popal; the flags are those of the final
// "add esi, 0x41c" (esi = 0x00675958).
void __cdecl FUN_00424814(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t record = 0x0066fef0u;
    for (uint32_t cars = 0x16u; cars != 0u; --cars, record += 0x41cu) {
        uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        c.esi = record;
        const uint32_t changed = GP4_FIELD(uint32_t, car, 0x44) ^ GP4_FIELD(uint32_t, car, 0x40);
        if (changed == 0u) {
            continue;
        }
        c.eax = changed;
        c.ebp = changed;
        GP4_GLOBAL(uint32_t, 0x00639638) = GP4_FIELD(uint32_t, car, 0x48);
        c.ecx = 0x17u;
        for (;;) {
            gp4::call_regs(0x004447b2, c);
            if ((c.eflags & 1u) != 0u) {
                break;
            }
            if (c.eax == 0u) {
                break;
            }
            GP4_GLOBAL(uint32_t, 0x00639634) = c.eax;
            c.edx = ~c.eax;
            c.ebp &= c.edx;
            gp4::call_regs(0x00424d65, c);
            if ((c.eflags & 1u) != 0u) {
                break;
            }
            if (c.ebp == 0u) {
                break;
            }
            c.ecx -= 1u;
            if (c.ecx == 0u) {
                break;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(0x0066fef0u + 21u * 0x41cu, 0x41cu);
}
GP4_IMPL(0x00424814, FUN_00424814, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+list_0066fe3c+cars_0066fef0+state_00437888+bytes_0063bcfc+state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+state_0043c4d9+state_00424814+cars_00424814")
