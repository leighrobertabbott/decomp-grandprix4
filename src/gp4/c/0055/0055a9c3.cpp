#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_008943a0 {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
};

static inline uint8_t setFlag08(uint32_t record) {
    Unknown_008943a0* rec = reinterpret_cast<Unknown_008943a0*>(record);
    rec->Flags92 = static_cast<uint8_t>(rec->Flags92 | 8u);
    return rec->Flags92;
}

// Sets bit 3 of byte +0x92 in the four track-segment records referenced by [0x00891500],
// [0x00866f00], [0x00866f04] and [0x00866ef8] (in that order, so aliased pointers behave as in the
// original). Leaves ebx = [0x00866ef8] and the flags of the last "or byte [ebx+0x92], 8".
// FUNCTION: GP4 0x0055a9c3
void __cdecl FUN_0055a9c3(gp4::Regs* r) {
    setFlag08(GP4_GLOBAL(uint32_t, 0x00891500));
    setFlag08(GP4_GLOBAL(uint32_t, 0x00866f00));
    setFlag08(GP4_GLOBAL(uint32_t, 0x00866f04));
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00866ef8);
    const uint8_t value = setFlag08(last);
    r->ebx = last;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(value));
}
GP4_IMPL(0x0055a9c3, FUN_0055a9c3, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=layout=ring_0040aec5")
