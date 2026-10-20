#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: index = ([0x007c31f2] & 0x3f) - 1 goes to the word [0x007c2f1c] and the dword
// 0x007c30d4[index] to [0x007c2f20]. Every GPR is restored; the flags are those of the last flag
// writer, `add esi, 0x007c30d4` (esi = index * 4).
// FUNCTION: GP4 0x00548e0b
void __cdecl FUN_00548e0b(gp4::Regs* r) {
    const uint32_t index = (GP4_GLOBAL(uint8_t, 0x007c31f2) & 0x3fu) - 1u;
    GP4_GLOBAL(uint16_t, 0x007c2f1c) = static_cast<uint16_t>(index);
    const uint32_t offset = index << 2;
    const uint32_t entry = offset + 0x007c30d4u;
    const uint32_t arith = gp4::flags::add32(offset, 0x007c30d4u);
    GP4_GLOBAL(uint32_t, 0x007c2f20) = GP4_GLOBAL(uint32_t, entry);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00548e0b, FUN_00548e0b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
