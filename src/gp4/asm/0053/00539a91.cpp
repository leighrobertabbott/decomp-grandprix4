#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item flag handler for the command in edx (esi = the item). Command 1 sets bit 6 of the item
// flag byte +0x1f and then bit 5 according to the level byte [0x007ab19d] (>= 3 sets it, below 3
// clears it); edx comes back 0 and the flags are those of that last and/or on the byte. Every
// other command is handled by the value handler 0x00538e1a (registers and flags as it leaves them).
// FUNCTION: GP4 0x00539a91
void __cdecl FUN_00539a91(gp4::Regs* r) {
    if (r->edx != 1u) {
        gp4::call_regs(0x00538e1a, *r);
        return;
    }
    uint8_t flags = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x1f) | 0x40u);
    GP4_FIELD(uint8_t, r->esi, 0x1f) = flags;
    if (GP4_GLOBAL(uint8_t, 0x007ab19d) < 3u) {
        flags = static_cast<uint8_t>(flags & 0xdfu);
    } else {
        flags = static_cast<uint8_t>(flags | 0x20u);
    }
    GP4_FIELD(uint8_t, r->esi, 0x1f) = flags;
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags));
}
GP4_IMPL(0x00539a91, FUN_00539a91, "regs(eax:u32, edx:u32{1,1,1,5,5,0,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
