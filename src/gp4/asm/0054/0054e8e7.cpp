#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame item of a record: unless bit 0x10 of the flag byte +0x1f is set (then the flags of that `test` are the only
// effect), loads ebx/eax/ebp/ecx from the words +4/+6/+8/+0xa (ebp and ecx get 6 added), calls the empty stub
// 0x54a1d7, inserts the entry in the sorted pool list with 0x549db9, and calls the empty stub 0x531b3a inside a
// pushal/popal pair (ebx/eax are bumped by 1 or 2 there for the stub only, popal restores them). Then edx grows by 2
// (bit 0x20), or, otherwise, by 1 when the id byte +0xc equals the selection byte [0x82fd45] (ebx/eax are bumped by
// one first when bit 0x80 is set), and the empty stub 0x531b35 is called. The flags are those of the last flag
// writer; the stc/clc epilogue only replaces CF of the `cmp ah, [0x82fd45]`.
// FUNCTION: GP4 0x0054e8e7
void __cdecl FUN_0054e8e7(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = gp4::flags::logic8(static_cast<uint32_t>(rec[0x1f] & 0x10u));
    if ((rec[0x1f] & 0x10u) == 0u) {
        uint32_t ebx = *reinterpret_cast<const uint16_t*>(rec + 4);
        uint32_t eax = *reinterpret_cast<const uint16_t*>(rec + 6);
        uint32_t ebp = *reinterpret_cast<const uint16_t*>(rec + 8) + 6u;
        uint32_t ecx = *reinterpret_cast<const uint16_t*>(rec + 10) + 6u;
        uint32_t edx = r->edx;

        gp4::cdecl_<0x0054a1d7, void>();

        gp4::Regs c = *r;
        c.eax = eax;
        c.ebx = ebx;
        c.ebp = ebp;
        c.ecx = ecx;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00549db9, c);

        gp4::cdecl_<0x00531b3a, void>();

        if ((rec[0x1f] & 0x20u) != 0u) {
            flags = gp4::flags::add32(edx, 2u);
            edx += 2u;
        } else {
            flags = gp4::flags::logic8(static_cast<uint32_t>(rec[0x1f] & 0x80u));
            if ((rec[0x1f] & 0x80u) != 0u) {
                ebx += 1u;
                flags = gp4::flags::add32(eax, 1u);
                eax += 1u;
            }
            const uint8_t id = rec[0x0c];
            const uint8_t selected = GP4_GLOBAL(uint8_t, 0x0082fd45);
            flags = gp4::flags::sub8(id, selected) & ~1u;
            if (id == selected) {
                flags = gp4::flags::add32(edx, 1u);
                edx += 1u;
            }
        }
        gp4::cdecl_<0x00531b35, void>();

        r->eax = eax;
        r->ebx = ebx;
        r->ecx = ecx;
        r->edx = edx;
        r->ebp = ebp;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e8e7, FUN_0054e8e7, "regs(esi:ptr[0x40]:layout=item_0054e8e7, eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=list_00549db9+item_0054e8e7")
