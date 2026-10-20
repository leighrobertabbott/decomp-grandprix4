#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041d9eb
// Chooses a scaling routine. When the byte at 0x00638d60 is not 0xff, 0x0041d96d returns a code in
// eax: 0 falls through to the default 0x0041d9b6; 4 runs 0x0041d996 and sets byte 0x00625ab8 to
// 0xff (edx = 4); any other value runs 0x0041d9a6 and sets byte 0x00625ab9 to 0xff (edx = 3).
// The default path (byte == 0xff or code 0) runs 0x0041d9b6 with edx = 0. eax/ecx and the flags
// are those of the routine that ran; edi is the record 0x0041d9b6 reads.
void __cdecl FUN_0041d9eb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if (GP4_GLOBAL(uint8_t, 0x00638d60) != 0xffu) {
        gp4::call_regs(0x0041d96d, c);
        if (c.eax != 0u) {
            if (c.eax != 4u) {
                gp4::call_regs(0x0041d9a6, c);
                c.edx = 3;
                GP4_GLOBAL(uint8_t, 0x00625ab9) = 0xff;
            } else {
                gp4::call_regs(0x0041d996, c);
                c.edx = 4;
                GP4_GLOBAL(uint8_t, 0x00625ab8) = 0xff;
            }
            *r = c;
            return;
        }
    }

    gp4::call_regs(0x0041d9b6, c);
    c.edx = 0;
    *r = c;
}
GP4_IMPL(0x0041d9eb, FUN_0041d9eb, "regs(edi:ptr[0x6d]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041d996+state_0041d9b6+mesh_00453330+state_0041d9eb")
