#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = table[esi]: esi is an index into the dword table at 0x00835846 (unaligned base).
// The flags are those of the `add esi, 0x835846` (the later mov leaves them alone).
// FUNCTION: GP4 0x0054d1ad
void __cdecl FUN_0054d1ad(gp4::Regs* r) {
    const uint32_t scaled = r->esi << 2;
    const uint32_t addr = scaled + 0x00835846u;
    const uint32_t flags = gp4::flags::add32(scaled, 0x00835846u);
    r->esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(addr));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054d1ad, FUN_0054d1ad, "regs(esi:u32[0..0x3ff]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
