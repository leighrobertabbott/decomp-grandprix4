#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t SlotWord(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

// `pushal`: the slot word 0x350 of the table at 0x009e1b20 (0xffff = none) is a byte offset into the
// pointer table 0x006975ec; the 64 KiB map it points to is copied (forward byte copy, rep movsb) to
// 0x009bf394. Every GPR is restored; the flags are those of `cmp bx, -1` when the slot is empty,
// otherwise of `add ebx, 0x6975ec`.
// FUNCTION: GP4 0x0051c746
void __cdecl FUN_0051c746(gp4::Regs* r) {
    const uint32_t slot = SlotWord(0x350u);
    uint32_t flags = gp4::flags::sub16(slot, 0xffffu);
    if (slot != 0xffffu) {
        flags = gp4::flags::add32(slot, 0x006975ecu);
        const uint8_t* src = *reinterpret_cast<const uint8_t* const*>(static_cast<uintptr_t>(0x006975ecu + slot));
        uint8_t* dst = GP4_ARRAY(uint8_t, 0x009bf394);
        for (uint32_t i = 0; i < 0x10000u; ++i) {
            dst[i] = src[i];
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051c746, FUN_0051c746, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tex_0051c746; wall=1800")
