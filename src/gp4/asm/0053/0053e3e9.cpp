#include <gp4/gp4.h>
#include <gp4/flags.h>

// Lays out the edi dialog record (callers: 0x84c830 / 0x84c8a4 / 0x84c93e with eax = 1 / 2 / 3
// buttons). Minimum width eax * 0xa0 - 0x40 -> 0x82fec4; looks up the title text (esi = edx through
// 0x0054d1ad) and measures it with the stub 0x0054d6f4 (which leaves the width in ebx and the
// height in eax); both are rounded up to even, the width raised to the minimum, stored to
// 0x82febc / 0x82fec0, and the box centred in 800 x 700 (x 0x82feb4, y 0x82feb8). The rectangle
// x, y, x + w + 0x3f, y + h + 0x3f is written after the zero-word terminator of the 6-byte list at
// +0x28, the text block +0x18 gets x + 0x20, y + 0x20. Then the body text (index +0x14) is
// looked up and the font sizes 0x82fec8..0x82feca are tried until the measured width (the stubs
// leave eax = the size byte, never above 0x1f4) fits: the first size is stored to +0x10.
// FUNCTION: GP4 0x0053e3e9
void __cdecl FUN_0053e3e9(gp4::Regs* r) {
    uint8_t* dialog = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    r->eax = r->eax * 0xa0u - 0x40u;
    GP4_GLOBAL(uint32_t, 0x0082fec4) = r->eax;
    r->esi = r->edx;
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x0054d6f4, *r);

    uint32_t width = (r->ebx + 1u) & 0xfffeu;
    const uint32_t height = (r->eax + 1u) & 0xfffeu;
    if (width < GP4_GLOBAL(uint32_t, 0x0082fec4)) {
        width = GP4_GLOBAL(uint32_t, 0x0082fec4);
    }
    GP4_GLOBAL(uint32_t, 0x0082febc) = width;
    GP4_GLOBAL(uint32_t, 0x0082fec0) = height;
    GP4_GLOBAL(int32_t, 0x0082feb4) = static_cast<int32_t>(0x320u - (width + 0x40u)) >> 1;
    GP4_GLOBAL(int32_t, 0x0082feb8) = static_cast<int32_t>(0x2bcu - (height + 0x60u)) >> 1;

    uint8_t* entry = dialog + 0x28;
    while (*reinterpret_cast<uint16_t*>(entry) != 0u) {
        entry += 6;
    }
    entry += 2;
    const uint32_t left = GP4_GLOBAL(uint32_t, 0x0082feb4);
    const uint32_t top = GP4_GLOBAL(uint32_t, 0x0082feb8);
    *reinterpret_cast<uint16_t*>(entry + 2) = static_cast<uint16_t>(left);
    *reinterpret_cast<uint16_t*>(entry + 4) = static_cast<uint16_t>(top);
    *reinterpret_cast<uint16_t*>(entry + 6) =
        static_cast<uint16_t>(left + GP4_GLOBAL(uint32_t, 0x0082febc) + 0x3fu);
    *reinterpret_cast<uint16_t*>(entry + 8) =
        static_cast<uint16_t>(top + GP4_GLOBAL(uint32_t, 0x0082fec0) + 0x3fu);

    uint8_t* text = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, dialog, 0x18)));
    *reinterpret_cast<uint16_t*>(text + 4) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x0082feb4) + 0x20u);
    r->eax = GP4_GLOBAL(uint32_t, 0x0082feb8) + 0x20u;
    *reinterpret_cast<uint16_t*>(text + 6) = static_cast<uint16_t>(r->eax);

    r->esi = GP4_FIELD(uint32_t, dialog, 0x14);
    gp4::call_regs(0x0054d1ad, *r);

    uint8_t* size = GP4_ARRAY(uint8_t, 0x0082fec8);
    uint32_t arith;
    for (;;) {
        r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(size));
        r->eax = *size;
        gp4::call_regs(0x0054cb71, *r);
        gp4::cdecl_<0x0054d6f2, void>();
        arith = gp4::flags::sub32(r->eax, 0x1f4u);
        if (r->eax <= 0x1f4u) {
            break;
        }
        arith = gp4::flags::sub32(r->ebx, 0x0082fecau);
        if (r->ebx == 0x0082fecau) {
            break;
        }
        ++size;
    }
    r->eax = *size;
    dialog[0x10] = static_cast<uint8_t>(r->eax);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053e3e9, FUN_0053e3e9, "regs(eax:u32, ebx:u32, edx:u32[0..0x3ff], edi:u32{=0x84c830,0x84c8a4,0x84c93e}) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00835846+dlg_0053e3e9")
