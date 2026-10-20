#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c536
// `push edx ; sar eax,9 ; sar edx,9 ; call 0x0040c43b ; pop edx ; ret`: scales the two
// coordinates down by 512 and looks the cell class up through 0x0040c43b (class byte in eax,
// flags of its final `and eax,0xff`). edx is restored.
void __cdecl FUN_0040c536(gp4::Regs* r) {
    const uint32_t savedEdx = r->edx;
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(r->eax) >> 9);
    r->edx = static_cast<uint32_t>(static_cast<int32_t>(r->edx) >> 9);
    gp4::call_regs(0x0040c43b, *r);
    r->edx = savedEdx;
}
GP4_IMPL(0x0040c536, FUN_0040c536, "regs(eax:u32, ebx:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=map_0040c43b")
