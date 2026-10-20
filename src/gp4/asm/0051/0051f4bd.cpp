#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills a (ecx>>8) wide by (edx>>8) high byte rectangle with al, centred on
// (edi, ebx), in the 8-bit surface at [0x00874ee4] with row pitch [0x008751c4].
// Every GPR is restored (pushal/popal); only the arithmetic flags are an output.
// FUNCTION: GP4 0x0051f4bd
void __cdecl FUN_0051f4bd(gp4::Regs* r) {
    const uint32_t fill = r->eax & 0xffu;
    const uint32_t w = r->ecx >> 8;
    const uint32_t h = r->edx >> 8;
    uint32_t flags;

    if (w == 0) {
        // flags of `shr esi, 8` with a zero result: CF = bit 7 of ecx, ZF = PF = 1
        flags = ((r->ecx >> 7) & 1u) | gp4::flags::ZF | gp4::flags::PF;
    } else {
        const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x008751c4);
        const uint32_t x0 = r->edi - (w >> 1);
        const uint32_t y0 = r->ebx - (h >> 1);
        uint32_t row = y0 * pitch + x0 + GP4_GLOBAL(uint32_t, 0x00874ee4);
        if (h == 0) {
            flags = gp4::flags::logic32(0u);
        } else {
            uint32_t last = row;
            for (uint32_t i = 0; i < h; i++) {
                uint8_t* p = (uint8_t*)(uintptr_t)row;
                for (uint32_t j = 0; j < w; j++) p[j] = (uint8_t)fill;
                last = row;
                row += pitch;
            }
            // last iteration: `add edi, pitch` then `dec ebp` (1 -> 0, CF kept)
            flags = gp4::flags::dec32(1u, gp4::flags::add32(last, pitch));
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f4bd, FUN_0051f4bd, "regs(eax:u32, ebx:u32[8..24], ecx:u32[0..0x2000], edx:u32[0..0x1000], edi:u32[16..48]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=surf_0051f4bd")
