#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x0055268f for the table at [ebx+0x54] (count-prefixed dwords, -1 = none; the table's entries are read
// as words). The index is 1 when neither bit of [ebx+0x60] is set, [ebx+0x5c] + 1 when bit 2 is set and
// 0x004015e2(count) (random 1..count) when only bit 3 is set; then clamped (signed < 1 -> 1, unsigned > count -> 1)
// and stored in [ebx+0x5c] and [edi+0x94]. The indexed entry's low word goes to [edi+0x58]; the words/dwords
// [ebx+0x34], [ebx+0x38], [ebx+0x3c], [ebx+0x3e], [ebx+0x32] are copied to [edi+0x60], +0x64, +0x68, +0x6a, +0x5a.
// ecx = the table, eax = ([ebx+0x38] & 0xffff0000) | word [ebx+0x32] (the `mov ax` stores only touch ax).
// Flags at ret: `cmp ecx, -1` when there is no table (nothing else changes), `test [ebx+0x60], 8` (zero) when the
// index is the constant 1, otherwise the final `cmp eax, [ecx]`.
// FUNCTION: GP4 0x005526eb
void __cdecl FUN_005526eb(gp4::Regs* r) {
    uint32_t table = GP4_FIELD(uint32_t, r->ebx, 0x54);
    uint32_t flags = gp4::flags::sub32(table, 0xffffffffu);
    r->ecx = table;
    if (table != 0xffffffffu) {
        const uint32_t mode = GP4_FIELD(uint32_t, r->ebx, 0x60);
        uint32_t index;
        bool clamp = true;
        if ((mode & 4u) != 0u) {
            index = GP4_FIELD(uint32_t, r->ebx, 0x5c) + 1u;
        } else if ((mode & 8u) != 0u) {
            r->eax = GP4_FIELD(uint32_t, table, 0);
            gp4::call_regs(0x004015e2, *r);
            index = r->eax;
            table = r->ecx;
        } else {
            index = 1u;
            clamp = false;
            flags = gp4::flags::logic32(mode & 8u);
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
        GP4_FIELD(uint32_t, r->ebx, 0x5c) = index;
        GP4_FIELD(uint32_t, r->edi, 0x94) = index;
        GP4_FIELD(uint16_t, r->edi, 0x58) = GP4_FIELD(uint16_t, table, index * 4u);
        GP4_FIELD(uint32_t, r->edi, 0x60) = GP4_FIELD(uint32_t, r->ebx, 0x34);
        const uint32_t high = GP4_FIELD(uint32_t, r->ebx, 0x38);
        GP4_FIELD(uint32_t, r->edi, 0x64) = high;
        GP4_FIELD(uint16_t, r->edi, 0x68) = GP4_FIELD(uint16_t, r->ebx, 0x3c);
        GP4_FIELD(uint16_t, r->edi, 0x6a) = GP4_FIELD(uint16_t, r->ebx, 0x3e);
        const uint16_t low = GP4_FIELD(uint16_t, r->ebx, 0x32);
        GP4_FIELD(uint16_t, r->edi, 0x5a) = low;
        r->eax = (high & 0xffff0000u) | low;
        r->ecx = table;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005526eb, FUN_005526eb, "regs(ebx:ptr[0x68]:layout=blk_005526eb, edi:ptr[0x98]:bytes, eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4+tab_0055268f")
