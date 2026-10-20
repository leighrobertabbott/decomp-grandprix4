#include <gp4/gp4.h>

// pushal ... popal: every register comes back; the flags are those of the final `cmp esi, 0x16` (equal: ZF and PF).
//
// For every driver esi = 0..21 whose flag byte 0x007c2a48[esi] is not negative and every circuit ebx = 0..16, the four
// 0x38-byte entries (ecx = 0..3) of the pair are initialised: 0x0051f82c stores the four bytes 0x007adad4..d7 at slots
// 0, 2, 1, 3 of the pair record, then 0x0051f837 returns the entry address (edi) and the template 0x007adad8 (entries
// 0 and 2) or 0x007adb10 (entries 1 and 3) is copied into it.
// FUNCTION: GP4 0x0043ba68
void __cdecl FUN_0043ba68(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (c.esi = 0; c.esi != 0x16u; ++c.esi) {
        if ((GP4_ARRAY(uint8_t, 0x007c2a48)[c.esi] & 0x80u) != 0u) {
            continue;
        }
        for (c.ebx = 0; c.ebx != 0x11u; ++c.ebx) {
            for (c.ecx = 0; c.ecx != 4u; ++c.ecx) {
                // pushal ... popal around the four slot stores: only the flags of the last call survive.
                gp4::Regs slot = c;
                slot.edx = 0u;
                slot.eax = c.esi;
                slot.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007adad4);
                gp4::call_regs(0x0051f82c, slot);
                slot.edx = 2u;
                slot.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007adad5);
                slot.eax = c.esi;
                gp4::call_regs(0x0051f82c, slot);
                slot.edx = 1u;
                slot.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007adad6);
                slot.eax = c.esi;
                gp4::call_regs(0x0051f82c, slot);
                slot.edx = 3u;
                slot.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007adad7);
                slot.eax = c.esi;
                gp4::call_regs(0x0051f82c, slot);
                c.eflags = slot.eflags;

                c.eax = c.esi;
                gp4::call_regs(0x0051f837, c);
                c.edi = c.eax;

                const uint8_t* source = GP4_ARRAY(uint8_t, 0x007adad8);
                if (c.ecx != 0u && c.ecx != 2u) {
                    source = GP4_ARRAY(uint8_t, 0x007adb10);
                }
                uint8_t* destination = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
                for (uint32_t i = 0; i < 0x38u; ++i) {
                    destination[i] = source[i];
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043ba68, FUN_0043ba68, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=codes_0043ba12; wall=3600")
