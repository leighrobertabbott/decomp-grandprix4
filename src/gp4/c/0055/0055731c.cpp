#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) void StoreSlotWord(uint32_t index, uint16_t value) {
    GP4_ARRAY(uint16_t, 0x006f3d94)[index] = value;
}

// Record reader: esi points at a pair of words (slot, value). When the mode byte 0x0086857c is 1 the
// value is stored in word ((slot - 1) & 0xf) of the 16-word table 0x006f3d94, leaving eax = byte
// offset of that word and ebx = 0x006f3d94. Either way esi advances by 4; the flags are those of that
// `add esi, 4`.
// FUNCTION: GP4 0x0055731c
void __cdecl FUN_0055731c(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t slot = GP4_FIELD(uint16_t, esi, 0);
        const uint16_t value = GP4_FIELD(uint16_t, esi, 2);
        const uint32_t index = static_cast<uint32_t>(static_cast<uint16_t>(slot - 1u)) & 0xfu;
        r->eax = index << 1;
        r->ebx = 0x006f3d94u;
        StoreSlotWord(index, value);
    }
    r->esi = esi + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
}
GP4_IMPL(0x0055731c, FUN_0055731c, "regs(eax:u32, ebx:u32, esi:ptr[8]:bytes) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
