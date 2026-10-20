#include <gp4/gp4.h>
#include <gp4/flags.h>

// When edx is 2 or 3: dword [ebx] = max([ebx] - 1, 0) (signed), eax = the stored value, flags of
// the `sub eax, 1`. Otherwise only the flags of `cmp edx, 3` change.
// FUNCTION: GP4 0x00539c04
void __cdecl FUN_00539c04(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub32(r->edx, 3u);
    if (r->edx == 2u || r->edx == 3u) {
        uint32_t* cell = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx));
        const uint32_t value = *cell;
        flags = gp4::flags::sub32(value, 1u);
        const uint32_t result = (static_cast<int32_t>(value) >= 1) ? value - 1u : 0u;
        *cell = result;
        r->eax = result;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539c04, FUN_00539c04, "regs(edx:u32{2,3,0,1,4}, ebx:ptr[8]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
