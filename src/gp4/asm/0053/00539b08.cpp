#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for the command in edx (esi = the item), the increasing twin of 0x00539aba. Command
// 1 does nothing. While the byte [0x007aafd3] is not 0xff the plain increment handler 0x00539154 runs and its
// registers/flags flow out. Otherwise commands 2 and 3 take the byte cell `dst` (0x007ab756, or 0x007a93b2
// when [0x007ab1a0] is not 0xff), step it up by one - and again while the result equals the other cell - and
// store it unless it rose above the signed maximum in dword +0x26 of the item. edi = dst, eax = the stepped
// value, edx = 0; the flags are those of the last compare. Any other command leaves the flags of `cmp edx, 3`.
// FUNCTION: GP4 0x00539b08
void __cdecl FUN_00539b08(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command == 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
        return;
    }
    if (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0xffu) {
        gp4::call_regs(0x00539154, *r);
        return;
    }
    if (command != 2u && command != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 3u));
        return;
    }

    uint8_t* dst = &GP4_GLOBAL(uint8_t, 0x007ab756);
    uint8_t* other = &GP4_GLOBAL(uint8_t, 0x007a93b2);
    if (GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu) {
        uint8_t* t = dst;
        dst = other;
        other = t;
    }

    uint32_t value = *dst;
    const uint32_t otherValue = *other;
    uint32_t arith;
    for (;;) {
        value += 1u;
        const uint32_t maximum = GP4_FIELD(uint32_t, r->esi, 0x26);
        arith = gp4::flags::sub32(value, maximum);
        if (static_cast<int32_t>(value) > static_cast<int32_t>(maximum)) {
            break;
        }
        arith = gp4::flags::sub32(value, otherValue);
        if (value != otherValue) {
            *dst = static_cast<uint8_t>(value);
            break;
        }
    }

    r->eax = value;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539b08, FUN_00539b08, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}, edi:u32) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_00539b56")
