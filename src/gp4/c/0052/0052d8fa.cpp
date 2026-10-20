#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of the 0x34-byte binding entry; kept out of line so the table base 0x007ab5c6 stays visible as the
// operand of the original `add eax, 0x7ab5c6`.
static __declspec(noinline) uint32_t entryAddr(uint32_t scaledIndex, uint32_t base) {
    return scaledIndex + base;
}

// Loads the six key words of the binding entry [0x007ab544] (0x34-byte entries from 0x007ab5c6) into the working
// copy at 0x007ab55a (`rep movsw`; the reverse of 0x0052d927), then sets the flag byte [0x0082fd49] = 0xff and the
// byte [0x00824649] = 0x0f. Exit: eax = the entry address, esi / edi just past the copied words, ecx = 0; the
// flags are those of the `add eax, 0x7ab5c6`.
// FUNCTION: GP4 0x0052d8fa
void __cdecl FUN_0052d8fa(gp4::Regs* r) {
    const uint32_t scaled = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x007ab544)) * 0x34u;
    const uint32_t entry = entryAddr(scaled, 0x007ab5c6u);
    uint32_t source = entry;
    uint32_t target = 0x007ab55au;
    for (uint32_t left = 6u; left != 0u; --left) {
        GP4_GLOBAL(uint16_t, target) = GP4_GLOBAL(uint16_t, source);
        source += 2u;
        target += 2u;
    }
    GP4_GLOBAL(uint8_t, 0x0082fd49) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00824649) = 0x0fu;

    r->eax = entry;
    r->ecx = 0u;
    r->esi = source;
    r->edi = target;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, 0x007ab5c6u));
}
GP4_IMPL(0x0052d8fa, FUN_0052d8fa, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=presets_007ab5b4+sel_007ab544")
