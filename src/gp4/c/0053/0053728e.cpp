#include <gp4/gp4.h>

// FUNCTION: GP4 0x0053728e
// Copies the language setting bytes: [0x82974f] -> [0x7ab17c] and the sign-extended byte [0x829751] ->
// the word [0x7ad8ac]. Unless [0x826e60] is 0xff, the 12 bytes at 0x82973e are copied (rep movsw, 6 words)
// into each of the 8 0x34-byte preset entries at 0x7ab5c6 (entry + 0x12); esi/edi/ecx are left at the
// end of the last copy (esi = 0x82974a, edi = end of entry 7's field, ecx = 0). Finally preset
// [0x7ab544] is selected with 0x52b942 (eax = that byte), which restores every register and leaves
// the flags of its `add`.
void __cdecl FUN_0053728e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x007ab17c) = GP4_GLOBAL(uint8_t, 0x0082974f);
    GP4_GLOBAL(uint16_t, 0x007ad8ac) = static_cast<uint16_t>(static_cast<int16_t>(GP4_GLOBAL(int8_t, 0x00829751)));

    if (GP4_GLOBAL(uint8_t, 0x00826e60) != 0xffu) {
        uint32_t index = 0;
        do {
            const uint16_t* src = reinterpret_cast<const uint16_t*>(0x0082973eu);
            uint16_t* dst = reinterpret_cast<uint16_t*>(index * 0x34u + 0x007ab5c6u);
            for (uint32_t n = 6; n != 0; --n) {
                *dst++ = *src++;
            }
            c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
            c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
            c.ecx = 0;
            ++index;
        } while (index != 8u);
    }

    c.eax = GP4_GLOBAL(uint8_t, 0x007ab544);
    gp4::call_regs(0x0052b942, c);
    *r = c;
}
GP4_IMPL(0x0053728e, FUN_0053728e, "regs(ecx:u32, esi:u32, edi:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=presets_007ab5b4; gfields={0x0082973e*3/4:u32, 0x007ab544:u8[0..7], 0x00826e60:u8{0xff,0,1}}")
