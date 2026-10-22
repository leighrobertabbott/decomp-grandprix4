#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the id [0x00ae6ca4] up in the slot table (0x00541d06: eax = the slot address or 0). When there is a slot, the
// byte at [esi] (also left in dl) is stored at slot + 0x15 and the dword 0x0083000c is cleared. The flags are those
// of the `or eax, eax` that tests the lookup result.
// FUNCTION: GP4 0x005408b5
void __cdecl FUN_005408b5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
    gp4::call_regs(0x00541d06, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        const uint8_t value = GP4_FIELD(uint8_t, c.esi, 0);
        c.edx = (c.edx & 0xffffff00u) | value;
        GP4_FIELD(uint8_t, c.eax, 0x15) = value;
        GP4_GLOBAL(uint32_t, 0x0083000c) = 0u;
    }
    *r = c;
}
GP4_IMPL(0x005408b5, FUN_005408b5, "regs(ebx:u32, ecx:u32, edx:u32, esi:ptr[0x10]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+slotsel_00540884+host_00b1bb40")
