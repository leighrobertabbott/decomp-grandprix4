#include <gp4/gp4.h>
#include <gp4/flags.h>

// A menu item (esi) points at a value block through its dword +0xe (packed, unaligned): current
// value +0, upper limit +4, lower bound +8 (signed dwords). Advances the current value by one,
// clamped to the limit and measured from the lower bound: value = min(value + 1 + base, limit) -
// base, or 0 when that falls below the bound. The result is stored back and eax, ebx leave as the
// result and the block address, followed by the menu refresh 0x0053dc57 (esi = item) whose
// register results and flags are kept.
// FUNCTION: GP4 0x0053dd3a
void __cdecl FUN_0053dd3a(gp4::Regs* r) {
    const uint32_t block = GP4_FIELD(uint32_t, r->esi, 0x0e);
    const uint32_t base = GP4_FIELD(uint32_t, block, 8);
    const uint32_t limit = GP4_FIELD(uint32_t, block, 4);
    uint32_t value = GP4_FIELD(uint32_t, block, 0) + 1u + base;
    if (static_cast<int32_t>(value) > static_cast<int32_t>(limit)) {
        value = limit;
    }
    uint32_t result = value - base;
    if (static_cast<int32_t>(value) < static_cast<int32_t>(base)) {
        result = 0u;
    }
    GP4_FIELD(uint32_t, block, 0) = result;
    r->eax = result;
    r->ebx = block;
    gp4::call_regs(0x0053dc57, *r);
}
GP4_IMPL(0x0053dd3a, FUN_0053dd3a, "regs(eax:u32, ecx:u32, edx:u32, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+items_0053dc57+vals_0053dd3a")
