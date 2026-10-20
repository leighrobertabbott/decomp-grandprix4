#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053db52
// Menu navigation by the direction bits in the byte at 0x007aaffc. Only when bit 2 of the menu flag
// byte 0x0082fd6a is set (the bit is cleared first): the current item is found by following the
// chain that starts at [0x0082fd34] [0x0082fd45] times (esi; the `loop` never takes a zero count),
// then 0x0053dca6 follows its link at offset ebx, chosen by the first set bit of the key byte:
//   bit 2 -> 0x24, bit 3 -> 0x25, bit 0 -> 0x22, bit 1 -> 0x23 (none: nothing is called).
// dl keeps the key byte, ecx is 0 after the walk and esi the current item; the flags are the
// callee's, or those of the last `test dl, 2` / of `test byte [0x82fd6a], 4` on the early exits.
void __cdecl FUN_0053db52(gp4::Regs* r) {
    const uint8_t menuFlags = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    if ((menuFlags & 4u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(menuFlags & 4u));
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = static_cast<uint8_t>(menuFlags & 0xfbu);

    const uint32_t keys = GP4_GLOBAL(uint8_t, 0x007aaffc);
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t count = GP4_GLOBAL(uint8_t, 0x0082fd45);
    do {
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        count -= 1u;
    } while (count != 0u);

    r->edx = (r->edx & 0xffffff00u) | keys;
    r->esi = esi;
    r->ecx = count;

    uint32_t linkOffset;
    uint32_t tested;
    if ((keys & 4u) != 0u) {
        tested = keys & 4u;
        linkOffset = 0x24u;
    } else if ((keys & 8u) != 0u) {
        tested = keys & 8u;
        linkOffset = 0x25u;
    } else if ((keys & 1u) != 0u) {
        tested = keys & 1u;
        linkOffset = 0x22u;
    } else if ((keys & 2u) != 0u) {
        tested = keys & 2u;
        linkOffset = 0x23u;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(keys & 2u));
        return;
    }

    r->ebx = linkOffset;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(tested));
    gp4::call_regs(0x0053dca6, *r);
}
GP4_IMPL(0x0053db52, FUN_0053db52, "regs(eax:u32, ebx:u32, edx:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+items_0053dc57+links_0053dca6+sel_0053d8c5")
