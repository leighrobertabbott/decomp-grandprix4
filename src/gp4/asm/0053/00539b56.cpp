#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item "pair" value handler (esi = the item, edx = the command). Command 1 does nothing. While the
// byte [0x007aafd3] is not 0xff the plain handler 0x005394b1 runs and its registers/flags flow out.
// Otherwise the item's value cell (+0xe) is first pointed at the byte 0x007a93b2 (when the byte
// [0x007ab1a0] is not 0xff) for the call and then at 0x007ab756; afterwards the byte cell `dst`
// (0x007ab756, or 0x007a93b2 when [0x007ab1a0] is not 0xff) is compared with the other cell: when they
// are equal it is moved by -1, or by +1 when it equals the dword [item + 0x22]. edi = dst, ebx = the
// other cell, eax = the stored value, edx = 0; the flags are those of the last compare, or of the
// `add al, dl` after an equal compare.
// FUNCTION: GP4 0x00539b56
void __cdecl FUN_00539b56(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command == 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
        return;
    }

    const uint32_t item = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0xffu) {
        gp4::call_regs(0x005394b1, *r);
        return;
    }

    if (GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu) {
        GP4_FIELD(uint32_t, item, 0xe) = 0x007a93b2u;
        gp4::call_regs(0x005394b1, *r);
        GP4_FIELD(uint32_t, item, 0xe) = 0x007ab756u;
    } else {
        gp4::call_regs(0x005394b1, *r);
    }

    uint8_t* dst = &GP4_GLOBAL(uint8_t, 0x007ab756);
    uint8_t* other = &GP4_GLOBAL(uint8_t, 0x007a93b2);
    if (GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu) {
        uint8_t* t = dst;
        dst = other;
        other = t;
    }

    uint32_t value = *dst;
    uint32_t arith = gp4::flags::sub8(value, *other);
    if (value == *other) {
        uint32_t delta = 0xffu;
        if (value == GP4_FIELD(uint32_t, item, 0x22)) {
            delta = 1u;
        }
        arith = gp4::flags::add8(value, delta);
        value = (value + delta) & 0xffu;
    }
    *dst = static_cast<uint8_t>(value);

    r->eax = value;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(other));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539b56, FUN_00539b56, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,4,5,6,0,4,2,3}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}, edi:u32) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+flag4_005394b1+pair_00539b56")
