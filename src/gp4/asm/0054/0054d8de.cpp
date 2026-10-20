#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inserts room in front of a NUL-terminated string: the string at esi (length L from strlen
// 0x0054d8d0) is moved up by eax bytes (a backward byte copy of L + 1 bytes, so the NUL goes
// along). The move is refused when eax + L >= 0x100: CF = 1 (stc) and the flags are those of
// `cmp edx, 0x100`. Otherwise CF = 0 (clc) with the other flags of the last `inc ecx` (L -> L + 1)
// and the direction flag is cleared. pushal/popal restores every register.
// FUNCTION: GP4 0x0054d8de
void __cdecl FUN_0054d8de(gp4::Regs* r) {
    const uint32_t shift = r->eax;
    gp4::Regs c = *r;
    gp4::call_regs(0x0054d8d0, c);
    const uint32_t length = c.eax;
    const uint32_t total = shift + length;
    if (total >= 0x100u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(total, 0x100u) | gp4::flags::CF);
        return;
    }
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi + length));
    uint8_t* target = source + shift;
    for (uint32_t left = length + 1u; left != 0u; --left) {
        *target-- = *source--;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(length, 0u)) & ~0x400u;
}
GP4_IMPL(0x0054d8de, FUN_0054d8de, "regs(eax:u32[0..0x120], esi:ptr[0x120]:u8[0..3]) -> (cf, pf, af, zf, sf, of)")
