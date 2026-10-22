#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parser step: skips blanks (0x0052b302), one ':' or '=' separator (0x0052b310) and blanks again, stores
// the cursor at [0x0081d414] and looks the word up in the keyword table at edx with 0x0052b2c0
// (edi = 0x0081d5ec + 1, ecx = the byte at 0x0081d5ec). On a hit the table entry's low byte is stored at
// [0x00829777] and CF is cleared (the other flags are those of the lookup). On a miss the error text
// routine 0x0052b31c runs and the byte [0x0081d420] is set to 0xff; the flags are then the callee's.
// FUNCTION: GP4 0x0052b3d0
void __cdecl FUN_0052b3d0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0052b302, c);
    gp4::call_regs(0x0052b310, c);
    gp4::call_regs(0x0052b302, c);
    GP4_GLOBAL(uint32_t, 0x0081d414) = c.esi;
    c.ecx = GP4_GLOBAL(uint8_t, 0x0081d5ec);
    c.edi = 0x0081d5ecu + 1u;
    gp4::call_regs(0x0052b2c0, c);
    if ((c.eflags & gp4::flags::CF) != 0u) {
        gp4::call_regs(0x0052b31c, c);
        GP4_GLOBAL(uint8_t, 0x0081d420) = 0xffu;
    } else {
        GP4_GLOBAL(uint8_t, 0x00829777) = static_cast<uint8_t>(c.eax);
        c.eflags &= ~static_cast<uint32_t>(gp4::flags::CF);
    }
    *r = c;
}
GP4_IMPL(0x0052b3d0, FUN_0052b3d0, "regs(eax:u32, ebx:u32, ecx:u32, edx:ptr[0x30]:layout=kwtab_0052b2c0, esi:ptr[0x40]:u8{0x61,0x62,0x63,0,0x20,0x09,0x3a,0x3d,0x7b}, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=script_all_00835846; gfields={0x0081d410:ptr[0x40]:u8{=0x41,0x5a,0x61,0x30,0x39,0x20,0x3a,0x3d,0}, 0x0081d5ec:u8, 0x0081d5fc*64/1:u8{=0,1,2,3,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x20,0x41,0x7f,0x80,0xff}}")
