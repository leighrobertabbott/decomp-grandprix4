#include <gp4/gp4.h>
#include <gp4/flags.h>

// Message handler "claim a driver key": esi points at a record whose first byte is a driver slot index.
// An index of 0x16 or more is ignored. Otherwise, when the key byte of that slot (0x007c2a1c + index) is
// still the free marker 0xff, the slot index of the current session handle [0x00ae6ca4] is looked up
// (0x00541d8d: -1 when no slot has the handle). A found index is released from the key table
// (0x00540698) and the code then falls into 0x0054086c, which stores it (al) as the key byte, claims it
// (0x005406bf), refreshes the flag bytes (0x00540648) and clears [0x0083000c]; with -1 the jump skips the
// store and runs the same tail directly. Registers and flags thread through the callees; on the early exits
// they are those of the index compare (ebx = index) or of the key compare (ebx = the key slot address).
// FUNCTION: GP4 0x00540845
void __cdecl FUN_00540845(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t index = GP4_FIELD(uint8_t, c.esi, 0);
    c.ebx = index;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(index, 0x16u));
    if (index < 0x16u) {
        c.ebx = index + 0x007c2a1cu;
        const uint8_t key = GP4_FIELD(uint8_t, c.ebx, 0);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(key, 0xffu));
        if (key == 0xffu) {
            c.eax = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
            gp4::call_regs(0x00541d8d, c);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0xffffffffu));
            if (c.eax != 0xffffffffu) {
                gp4::call_regs(0x00540698, c);
                gp4::call_regs(0x0054086c, c);
            } else {
                gp4::call_regs(0x005406bf, c);
                gp4::call_regs(0x00540648, c);
                GP4_GLOBAL(uint32_t, 0x0083000c) = 0u;
            }
        }
    }
    *r = c;
}
GP4_IMPL(0x00540845, FUN_00540845, "regs(esi:ptr[0x40]:u8{0,1,2,3,4,5,6,7,8,0x14,0x15,0x16,0x17,0xff}) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=blocks_0054045b+drv22_0052d402+keys_007c2a1c+slots_00541d5a+kind_00541cdb+slotsel_00540884")
