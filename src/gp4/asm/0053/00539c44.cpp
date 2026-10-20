#include <gp4/gp4.h>
#include <gp4/flags.h>

// When edx is 2 or 3: sets the value at the block ebx (current +0, upper limit +4, lower bound +8,
// signed dwords) to a clamped step: value = min(current + dword [esi + 0x26] + bound, limit) -
// bound, or 0 when that falls below the bound. The result is stored at [ebx] and left in eax. The
// flags are those of `sub eax, [ebx + 8]` on that path, else of `cmp edx, 3`.
// FUNCTION: GP4 0x00539c44
void __cdecl FUN_00539c44(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub32(r->edx, 3u);
    if (r->edx == 2u || r->edx == 3u) {
        const uint32_t block = r->ebx;
        const uint32_t base = GP4_FIELD(uint32_t, block, 8);
        const uint32_t limit = GP4_FIELD(uint32_t, block, 4);
        uint32_t value = GP4_FIELD(uint32_t, block, 0) + GP4_FIELD(uint32_t, r->esi, 0x26) + base;
        if (static_cast<int32_t>(value) > static_cast<int32_t>(limit)) {
            value = limit;
        }
        uint32_t result = value - base;
        flags = gp4::flags::sub32(value, base);
        if (static_cast<int32_t>(value) < static_cast<int32_t>(base)) {
            result = 0u;
        }
        GP4_FIELD(uint32_t, block, 0) = result;
        r->eax = result;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539c44, FUN_00539c44, "regs(eax:u32, edx:u32{2,3,0,1,4}, ebx:ptr[0x10]:bytes, esi:ptr[0x30]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
