#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call target so the compiler keeps two distinct call sites.
static __declspec(noinline) void CallCarSlot(gp4::Regs& c) {
    gp4::call_regs(0x00408309, c);
}
static __declspec(noinline) void CallOrderSearch(gp4::Regs& c) {
    gp4::call_regs(0x00516d07, c);
}

// Resets the four state bytes 0x88fd94..97 and records the slot number of the car record in esi
// in [0x866124]: the value of 0x408309 when the mode byte [0x7c2ea9] is 0x80, otherwise the order
// search 0x516d07 result plus one in ax (the upper half of eax is kept). [0x88fe14] is stored 0 and
// then 0xffffffff, bit 1 of the record byte +0x88 is cleared and [0x954169] is set to 0xff. eax is
// the stored value; the flags are those of the byte `and`.
// FUNCTION: GP4 0x005150e4
void __cdecl FUN_005150e4(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;

    gp4::Regs c = *r;
    uint32_t value;
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        CallCarSlot(c);
        value = c.eax;
    } else {
        CallOrderSearch(c);
        value = (c.eax & 0xffff0000u) | static_cast<uint16_t>(c.eax + 1u);
    }

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0u;
    GP4_GLOBAL(uint32_t, 0x00866124) = value;
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    const uint8_t updated = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x88) & 0xfdu);
    GP4_FIELD(uint8_t, r->esi, 0x88) = updated;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(updated));
}
GP4_IMPL(0x005150e4, FUN_005150e4, "regs(eax:u32, esi:ptr[0x420]:u8{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,0x41,0x96,0xc5,0x7f}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958")
