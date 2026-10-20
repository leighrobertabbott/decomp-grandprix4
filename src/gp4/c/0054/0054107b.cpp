#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the items of the current menu list ([0x0082fd34] = header, first item = [header], next = dword +0)
// that precede the first "plain" item: eax = 0 for no menu; otherwise the walk steps item by item and stops
// at the null link or at the first item whose flag byte +0x1f has none of bits 0x18 and whose byte +0x1e has
// bit 0x80 clear; eax is the number of steps before that item. esi is saved and restored; every exit leaves
// the flags of a zero logic result (`or esi, esi` on null, `test` with no bit set).
// FUNCTION: GP4 0x0054107b
void __cdecl FUN_0054107b(gp4::Regs* r) {
    uint8_t* node = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0082fd34)));
    uint32_t count = 0u;
    if (node != nullptr) {
        count = 0xffffffffu;
        for (;;) {
            ++count;
            node = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, node, 0)));
            if (node == nullptr) {
                break;
            }
            if ((GP4_FIELD(uint8_t, node, 0x1f) & 0x18u) != 0u) {
                continue;
            }
            if ((GP4_FIELD(uint8_t, node, 0x1e) & 0x80u) != 0u) {
                continue;
            }
            break;
        }
    }
    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0054107b, FUN_0054107b, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+items_0053dc57+sel_0053ccc7")
