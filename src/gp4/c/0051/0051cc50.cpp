#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keeps five pairs of slots of the 16-bit slot table 0x009e1b20 (0xffff = unused) consistent: the
// pair's two indices are the dwords at 0x009e1798 + k*8 and 0x009e179c + k*8; when either slot is
// 0xffff both are set to 0xffff. eax/edx end as the last pair's indices, ecx as 5; the flags are
// those of the final `cmp ecx, 5`.
// FUNCTION: GP4 0x0051cc50
void __cdecl FUN_0051cc50(gp4::Regs* r) {
    uint16_t* const slots = GP4_ARRAY(uint16_t, 0x009e1b20);
    uint32_t first = 0u;
    uint32_t second = 0u;
    uint32_t k = 0u;
    do {
        first = GP4_ARRAY(uint32_t, 0x009e1798)[k * 2u];
        second = GP4_ARRAY(uint32_t, 0x009e179c)[k * 2u];
        if (slots[first] == 0xffffu || slots[second] == 0xffffu) {
            slots[first] = 0xffffu;
            slots[second] = 0xffffu;
        }
        ++k;
    } while (k != 5u);

    r->eax = first;
    r->ecx = k;
    r->edx = second;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(k, 5u));
}
GP4_IMPL(0x0051cc50, FUN_0051cc50, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=slots_0051cc50")
