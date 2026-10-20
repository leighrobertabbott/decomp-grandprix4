#include <gp4/gp4.h>
#include <gp4/flags.h>

// strlen: eax = length of the NUL-terminated string at esi (esi is preserved). The flags are those
// of the final `sub eax, esi` (end pointer minus start pointer).
// FUNCTION: GP4 0x0054d8d0
void __cdecl FUN_0054d8d0(gp4::Regs* r) {
    const uint8_t* start = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* end = start;
    while (*end != 0u) {
        ++end;
    }
    const uint32_t length = static_cast<uint32_t>(end - start);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->esi + length, r->esi));
    r->eax = length;
}
GP4_IMPL(0x0054d8d0, FUN_0054d8d0, "regs(esi:ptr[0x40]:u8[0..3]) -> (eax, cf, pf, af, zf, sf, of)")
