#include <gp4/gp4.h>

// Round-robin search over the 22 car records (0x0066fef0, stride 0x41c): starting after the record at
// 0x0062f8b4 (or at the first record when it is -1), find the next record whose byte +0x7c has bit 7
// clear. The record pointer is stored in 0x0062f8b4, or -1 when all 22 have bit 7 set. esi / ecx are left
// as the loop ends; the flags are those of the last `test byte [esi+0x7c], 0x80`.
// FUNCTION: GP4 0x00445e43
void __cdecl FUN_00445e43(gp4::Regs* r) {
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0062f8b4);
    uint32_t ecx = 0x16u;
    uint32_t flags;
    if (esi == 0xffffffffu) {
        esi = 0x0066fef0u;
    }
    for (;;) {
        esi += 0x41cu;
        if (esi >= 0x00675958u) {
            esi = 0x0066fef0u;
        }
        const uint32_t masked = static_cast<uint32_t>(*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi + 0x7cu))) & 0x80u;
        flags = (masked == 0u ? 0x44u : 0x80u);
        if (masked == 0u) {
            GP4_GLOBAL(uint32_t, 0x0062f8b4) = esi;
            break;
        }
        ecx -= 1u;
        if (ecx == 0u) {
            GP4_GLOBAL(uint32_t, 0x0062f8b4) = 0xffffffffu;
            break;
        }
    }
    r->ecx = ecx;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445e43, FUN_00445e43, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0040fa51+cursor_0062f8b4")
