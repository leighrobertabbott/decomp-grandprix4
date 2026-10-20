#include <gp4/gp4.h>
#include <gp4/flags.h>

// Message handler (pushal ... popal): esi points at a packet whose words at +4, +6, +8 and +0xa are queued as
// ebx, eax, ebp and ecx through the sorted-pool insert 0x00549db9, then 0x0052800e is run with edx = 4 * [0x007ad894]
// + [0x00838e94] + 0x236. Every register is restored; the flags are those the last callee (0x0052800e) leaves.
// FUNCTION: GP4 0x0054f26b
void __cdecl FUN_0054f26b(gp4::Regs* r) {
    const uint8_t* packet = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.ebx = *reinterpret_cast<const uint16_t*>(packet + 4);
    c.eax = *reinterpret_cast<const uint16_t*>(packet + 6);
    c.ebp = *reinterpret_cast<const uint16_t*>(packet + 8);
    c.ecx = *reinterpret_cast<const uint16_t*>(packet + 10);
    gp4::call_regs(0x00549db9, c);

    const uint32_t scaled = GP4_GLOBAL(uint32_t, 0x007ad894) << 2;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00838e94);
    c.edx = scaled + base + 0x236u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(scaled + base, 0x236u));
    gp4::call_regs(0x0052800e, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054f26b, FUN_0054f26b, "regs(esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+trk_007ad894+slots_005388ed+cells_00527eae+slot8_005388ba+file_0054f26b")
