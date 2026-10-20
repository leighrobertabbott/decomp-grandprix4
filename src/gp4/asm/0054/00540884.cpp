#include <gp4/gp4.h>
#include <gp4/flags.h>

// Handler for a "set kind of the current slot" message. edi points at a flag byte and esi at the new kind byte.
// When the flag byte is zero the message is only honoured if some in-use slot of the 22-slot table at 0x00b1b8d8
// has a kind other than 1 (0x00541cdb counts them). The slot whose id is the dword at 0x00ae6ca4 is then looked up
// by 0x00541d06 (eax = slot address, 0 when absent); when it exists its kind byte (+0x14) becomes [esi] and the
// dword at 0x0083000c is cleared. Registers and flags are whatever the callees leave, the flags being those of
// the last `or eax, eax`.
// FUNCTION: GP4 0x00540884
void __cdecl FUN_00540884(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint8_t flag = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.edi));
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(flag, 0u));

    if (flag == 0u) {
        c.eax = 1u;
        gp4::call_regs(0x00541cdb, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax == 0u) {
            *r = c;
            return;
        }
    }

    c.eax = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
    gp4::call_regs(0x00541d06, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        const uint8_t kind = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
        c.edx = (c.edx & 0xffffff00u) | kind;
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.eax) + 0x14u) = kind;
        GP4_GLOBAL(uint32_t, 0x0083000c) = 0u;
    }
    *r = c;
}
GP4_IMPL(0x00540884, FUN_00540884, "regs(edx:u32, esi:ptr[0x40]:bytes, edi:ptr[0x40]:u8{0,0,1,0x20}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+slotsel_00540884")
