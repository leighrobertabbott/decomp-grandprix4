#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: for each of the ten slots k = 0..9 of the record edi (a 0x160-byte ring record),
// the slot's mask [0x00875d8c + 4k] is tested against ([edi - 0x134] | [edi + 0x2c]). A hit runs the
// point routine 0x00551227 (eax = -1 for k <= 4, else 0; esi = 2k; ebx / esi restored afterwards)
// and stores its x [0x00934df0], ecx and y [0x00934df4] at [esi + 4k + 0x00871cb0 / 0x00872ab0 /
// 0x008738b0] (esi = the caller's block offset); a miss stores zero in the three. popal restores
// every register; the flags are those of the last `cmp ebx, 0xa` (equal).
// FUNCTION: GP4 0x005520a0
void __cdecl FUN_005520a0(gp4::Regs* r) {
    const uint32_t base = r->esi;
    const uint32_t record = r->edi;

    gp4::Regs live = *r;
    live.st_in = 0;
    live.st_out = 0;
    live.st_xin_mask = 0;
    live.st_xout_mask = 0;

    for (uint32_t index = 0u; index != 10u; ++index) {
        const uint32_t mask = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record - 0x134u)) |
                              *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record + 0x2cu));
        uint32_t x = 0u;
        uint32_t y = 0u;
        uint32_t height = 0u;
        if ((GP4_ARRAY(uint32_t, 0x00875d8c)[index] & mask) != 0u) {
            live.eax = (static_cast<int32_t>(index) <= 4) ? 0xffffffffu : 0u;
            live.ebx = index;
            live.esi = index << 1;
            gp4::call_regs(0x00551227, live);
            live.ebx = index;
            live.esi = base;
            x = GP4_GLOBAL(uint32_t, 0x00934df0);
            height = live.ecx;
            y = GP4_GLOBAL(uint32_t, 0x00934df4);
        }
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(base + index * 4u + 0x00871cb0u)) = x;
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(base + index * 4u + 0x00872ab0u)) = height;
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(base + index * 4u + 0x008738b0u)) = y;
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(10u, 10u));
}
GP4_IMPL(0x005520a0, FUN_005520a0, "regs(esi:u32{=0,0x38,0x70,0xe0,0x540}, edi:rec[0x8943a0,0x160,1..7]) -> (cf, pf, af, zf, sf, of); globals=layout=geom_008943a0+span_00551227+mask_005520a0")
