#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips forward through the byte string at esi until a byte below 0x14 (a control code) is found
// and leaves esi pointing at that byte. al holds the control byte on exit (the rest of eax is
// untouched). The flags are those of the final `dec esi`: CF is still set from the `cmp al, 0x14`
// that ended the scan, the other flags describe the decremented pointer.
// FUNCTION: GP4 0x0054d3b6
void __cdecl FUN_0054d3b6(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t code;
    do {
        code = *p++;
    } while (code >= 0x14u);

    const uint32_t after = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    const uint32_t cmpFlags = gp4::flags::sub8(code, 0x14u);
    r->eax = (r->eax & 0xffffff00u) | code;
    r->esi = after - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(after, cmpFlags));
}
GP4_IMPL(0x0054d3b6, FUN_0054d3b6, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of)")
