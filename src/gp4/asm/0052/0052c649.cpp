#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Unknown_MenuItem26 {
    uint8_t Unknown00[0x26];
    uint32_t Limit26;
};
#pragma pack(pop)

// Menu value item handler keyed by edx. 1: only clears edx. 2 or 3: reads the item value at [ebx] (width from
// the item record esi, 0x00539944), adds one, keeps the sum when it does not wrap and is not above the
// limit dword [esi + 0x26] (unaligned), otherwise stores the limit, writes the value back (0x00539a13) and
// refreshes the dependent items with the pushal routine 0x0052bae5. Every other value does nothing. The flags
// are those of `cmp edx, 1` (edx = 1), the last callee (2, 3) or `cmp edx, 3`.
// FUNCTION: GP4 0x0052c649
void __cdecl FUN_0052c649(gp4::Regs* r) {
    const uint32_t event = r->edx;
    uint32_t flags = gp4::flags::sub32(event, 1u);
    if (event == 1u) {
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    flags = gp4::flags::sub32(event, 2u);
    bool handled = event == 2u;
    if (!handled) {
        flags = gp4::flags::sub32(event, 3u);
        handled = event == 3u;
    }
    if (!handled) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    gp4::call_regs(0x00539944, c);
    const uint32_t value = c.eax;
    const uint32_t limit = reinterpret_cast<const Unknown_MenuItem26*>(static_cast<uintptr_t>(c.esi))->Limit26;
    const uint32_t sum = value + 1u;
    uint32_t sum_flags = gp4::flags::add32(value, 1u);
    c.eax = sum;
    if ((sum_flags & gp4::CF) != 0u) {
        c.eax = limit;
    } else {
        sum_flags = gp4::flags::sub32(sum, limit);
        if (sum > limit) {
            c.eax = limit;
        }
    }
    c.eflags = gp4::flags::merge(c.eflags, sum_flags);
    gp4::call_regs(0x00539a13, c);
    gp4::call_regs(0x0052bae5, c);
    *r = c;
}
GP4_IMPL(0x0052c649, FUN_0052c649, "regs(edx:u32{1,2,3,2,3,0,5}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=set_0081d60f+tab_0081dc63+flags_0052bca6+script_0054d798+strs_0052c6be")
