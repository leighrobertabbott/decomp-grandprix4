#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00530162
// `pushal ... popal`: builds a render-state flag word in eax from the current device record (ebx is
// the record 0x00531803 selects, esi = 0x00843849 on entry to it) and leaves only the flags of the
// last flag writer: the `cmp` of [0x7aafd3] / [0x7ab020] against 0, or the final `or eax, 0xc`.
// The registers themselves are restored by the popal. The code after 0x005301d8 is the same routine
// (the lead excluded that tail as a separate function).
void __cdecl FUN_00530162(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = 0x00843849u;
    gp4::call_regs(0x00531803, c);
    gp4::call_regs(0x00531847, c);

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));

    uint32_t eax = ((c.eflags & gp4::flags::CF) == 0u) ? 0x10000u : 0xfffff0c4u;
    if ((GP4_FIELD(uint32_t, record, 0x10) & 0x1867f83cu) != 0u) {
        eax |= 0x20000u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007ad869) != 0u) {
        eax |= 0xc0u;
    } else {
        eax |= 0x4000000u;
    }
    if (GP4_GLOBAL(uint32_t, 0x007ad87c) != 0u) {
        eax |= 0xc0u;
    }
    if (GP4_FIELD(uint8_t, record, 0x24) == 0u) {
        eax |= 0xf00u;
    }
    if (GP4_FIELD(uint8_t, record, 0x25) == 0u) {
        eax |= 0xe00u;
    }

    const uint8_t first = GP4_GLOBAL(uint8_t, 0x007aafd3);
    uint32_t flags = gp4::flags::sub8(first, 0u);
    bool setBits = first != 0u;
    if (!setBits) {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x007ab020);
        flags = gp4::flags::sub8(second, 0u);
        setBits = second != 0u;
    }
    if (setBits) {
        eax |= 0xcu;
        flags = gp4::flags::logic32(eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00530162, FUN_00530162, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00531859+rsrec_007ad638+flag_007ab020")
