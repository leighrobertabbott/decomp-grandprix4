#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the next entry of a count-prefixed dword table ([ebx+0x50] = table, table[0] = count,
// entries 1..count) for the block ebx and stores it in [ebx+0x54] and [edi+0x90]; [ebx+0x58] keeps
// the chosen index. [ebx+0x50] == -1 means no table: [edi+0x90] = [ebx+0x54] only. The index is
//  - 1                 when neither bit of [ebx+0x60] is set,
//  - [ebx+0x58] + 1    when bit 0 is set (sequential),
//  - 0x004015e2(count) when only bit 1 is set (random 1..count),
// then clamped: signed < 1 -> 1, unsigned > count -> 1. eax = the entry, ecx = the table pointer.
// Flags at ret: `cmp ecx, -1` on the no-table path, `test [ebx+0x60], 2` (zero result) when the index
// is the constant 1, otherwise the final `cmp eax, [ecx]`.
// FUNCTION: GP4 0x0055268f
void __cdecl FUN_0055268f(gp4::Regs* r) {
    uint32_t table = GP4_FIELD(uint32_t, r->ebx, 0x50);
    uint32_t flags = gp4::flags::sub32(table, 0xffffffffu);
    r->ecx = table;
    if (table == 0xffffffffu) {
        const uint32_t value = GP4_FIELD(uint32_t, r->ebx, 0x54);
        GP4_FIELD(uint32_t, r->edi, 0x90) = value;
        r->eax = value;
    } else {
        const uint32_t mode = GP4_FIELD(uint32_t, r->ebx, 0x60);
        uint32_t index;
        bool clamp = true;
        if ((mode & 1u) != 0u) {
            index = GP4_FIELD(uint32_t, r->ebx, 0x58) + 1u;
        } else if ((mode & 2u) != 0u) {
            r->eax = GP4_FIELD(uint32_t, table, 0);
            gp4::call_regs(0x004015e2, *r);
            index = r->eax;
            table = r->ecx;
        } else {
            index = 1u;
            clamp = false;
            flags = gp4::flags::logic32(mode & 2u);
        }
        if (clamp) {
            if (static_cast<int32_t>(index) < 1) {
                index = 1u;
            }
            const uint32_t count = GP4_FIELD(uint32_t, table, 0);
            flags = gp4::flags::sub32(index, count);
            if (index > count) {
                index = 1u;
            }
        }
        GP4_FIELD(uint32_t, r->ebx, 0x58) = index;
        const uint32_t value = GP4_FIELD(uint32_t, table, index * 4u);
        GP4_FIELD(uint32_t, r->ebx, 0x54) = value;
        GP4_FIELD(uint32_t, r->edi, 0x90) = value;
        r->eax = value;
        r->ecx = table;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055268f, FUN_0055268f, "regs(ebx:ptr[0x68]:layout=blk_0055268f, edi:ptr[0x98]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4+tab_0055268f")
