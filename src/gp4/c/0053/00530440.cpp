#include <gp4/gp4.h>

// `call 0x00531803 ; mov eax, [0x007ad818] ; mov [ebx + 0x20], eax ; ret`: 0x00531803 selects the
// current record and returns it in ebx (with the flags of its last flag writer); the dword at
// 0x007ad818 is stored at record + 0x20 and left in eax.
// FUNCTION: GP4 0x00530440
void __cdecl FUN_00530440(gp4::Regs* r) {
    gp4::call_regs(0x00531803, *r);
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007ad818);
    r->eax = value;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx) + 0x20u) = value;
}
GP4_IMPL(0x00530440, FUN_00530440, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648+bits_007ad844")
