#include <gp4/gp4.h>
#include <gp4/flags.h>

// Offsets the two stored view coordinates (dwords 0x00833cc5 / 0x00833cc9) by ebx and builds a
// surface address: edi = lo32(((y + k) * scale)) + 2 * (x - k) + base, where x, y are the updated
// coordinates, k = [0x00833f80], scale = [0x0082224c], base = [0x00822258]. edx receives the high
// half of the product; eax and ebx are saved around the maths so they end as y and x. The flags
// are those of the final "add edi, [0x822258]".
// FUNCTION: GP4 0x0054d6b0
void __cdecl FUN_0054d6b0(gp4::Regs* r) {
    gp4::call_regs(0x0054d6f1, *r);

    const uint32_t delta = r->ebx;
    const uint32_t x = GP4_GLOBAL(uint32_t, 0x00833cc5) + delta;
    GP4_GLOBAL(uint32_t, 0x00833cc5) = x;
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x00833cc9) + delta;
    GP4_GLOBAL(uint32_t, 0x00833cc9) = y;

    const uint32_t k = GP4_GLOBAL(uint32_t, 0x00833f80);
    uint32_t doubled = x - k;
    doubled += doubled;
    const uint64_t product = (uint64_t)(y + k) * (uint64_t)GP4_GLOBAL(uint32_t, 0x0082224c);
    const uint32_t low = (uint32_t)product + doubled;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00822258);

    r->eax = y;
    r->ebx = x;
    r->edx = (uint32_t)(product >> 32);
    r->edi = low + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(low, base));
}
GP4_IMPL(0x0054d6b0, FUN_0054d6b0, "regs(ebx:u32) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
