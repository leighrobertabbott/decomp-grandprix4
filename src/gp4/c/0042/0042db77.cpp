#include <gp4/gp4.h>

// Flags of a 16-bit `cmp left, right` / `sub` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

struct Unknown_0042db77 {
    uint8_t Unknown00[0x90];
    uint16_t Unknown90;
    uint16_t Unknown92;
    uint16_t Unknown94;
    uint8_t Unknown96[0x11c - 0x96];
    uint16_t Unknown11C;
    uint16_t Unknown11E;
};

// FUNCTION: GP4 0x0042db77
void __cdecl FUN_0042db77(gp4::Regs* r) {
    const Unknown_0042db77* body = reinterpret_cast<const Unknown_0042db77*>(static_cast<uintptr_t>(r->ebx));
    const int16_t selector = GP4_GLOBAL(int16_t, 0x0060414e);
    uint16_t ax;

    if (selector == 0) {
        ax = body->Unknown94;
        const int8_t mode = GP4_GLOBAL(int8_t, 0x0060414c);
        if (mode != 0) {
            if (mode < 0) {
                const uint16_t low = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603baa));
                ax = static_cast<uint16_t>(static_cast<int16_t>(static_cast<uint16_t>(ax - body->Unknown11C)) >> 1);
                if (static_cast<int16_t>(ax) < static_cast<int16_t>(low)) {
                    ax = low;
                }
            } else {
                const uint16_t high = static_cast<uint16_t>(ax + GP4_GLOBAL(uint16_t, 0x00603baa));
                ax = static_cast<uint16_t>(static_cast<int16_t>(static_cast<uint16_t>(ax + body->Unknown11E)) >> 1);
                if (static_cast<int16_t>(ax) > static_cast<int16_t>(high)) {
                    ax = high;
                }
            }
        }
    } else {
        uint16_t dx = static_cast<uint16_t>(selector);
        if (selector < 0) {
            dx = static_cast<uint16_t>(0u - dx);
        }
        r->edx = (r->edx & 0xffff0000u) | dx;
        if (dx != 8u) {
            // stc after "cmp dx, 8"
            r->eflags = (r->eflags & ~0x8d5u) | subFlags16(dx, 8u) | 1u;
            return;
        }
        ax = GP4_GLOBAL(uint16_t, 0x00603ae0);
        if (selector < 0) {
            ax = static_cast<uint16_t>(0u - ax);
        }
        ax = static_cast<uint16_t>(ax + body->Unknown94);
    }

    const uint16_t left = ax;
    ax = static_cast<uint16_t>(ax - body->Unknown90);
    r->eax = (r->eax & 0xffff0000u) | ax;
    // "sub ax, [ebx+0x90]" then clc
    r->eflags = (r->eflags & ~0x8d5u) | (subFlags16(left, body->Unknown90) & ~1u);
}
GP4_IMPL(0x0042db77, FUN_0042db77, "regs(eax:u32, edx:u32, ebx:ptr[0x130]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
