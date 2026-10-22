#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: looks the handle [0x00b1bb40] up in the 22-slot table with 0x00541d8d (slot index in eax, or
// -1). For a hit, every one of the 22 flag bytes at 0x007c2a48 whose top two bits are exactly 10 (0x80) makes the
// matching key byte at 0x007c2a1c take al, the low byte of the slot index (the loop only changes ah). popal
// restores every register, so only the flags escape: those of `cmp eax, -1` on a miss, or of the final
// `or ecx, ecx` (ecx = 0) of the loop. The popal falls through into the bare `ret` function 0x00540268.
// FUNCTION: GP4 0x00540235
void __cdecl FUN_00540235(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.esi = 0x007c2a48u;
    c.edi = 0x007c2a1cu;
    c.eax = GP4_GLOBAL(uint32_t, 0x00b1bb40);
    gp4::call_regs(0x00541d8d, c);

    uint32_t arith = gp4::flags::sub32(c.eax, 0xffffffffu);
    if (c.eax != 0xffffffffu) {
        const uint8_t* flags = GP4_ARRAY(const uint8_t, 0x007c2a48);
        uint8_t* keys = GP4_ARRAY(uint8_t, 0x007c2a1c);
        for (uint32_t i = 0; i < 0x16u; ++i) {
            if ((flags[i] & 0xc0u) == 0x80u) {
                keys[i] = static_cast<uint8_t>(c.eax);
            }
        }
        arith = gp4::flags::logic32(0u);
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
    gp4::cdecl_<0x00540268, void>();
}
GP4_IMPL(0x00540235, FUN_00540235, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005401a2+slots_00541d5a+kind_00541cdb+slotsel_00540884")
