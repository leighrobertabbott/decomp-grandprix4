#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first of the 22 bytes at 0x7c2a48 whose low six bits equal al; eax = (its index) >> 1, or 0 when
// there is none (the original leaves ecx = 0x16 then). The flags are those of the final `shr eax, 1`
// (CF = bit 0 of the index, ZF/SF/PF of the result, OF = AF = 0). ecx, edx and esi are saved and restored.
// FUNCTION: GP4 0x005484f2
void __cdecl FUN_005484f2(gp4::Regs* r) {
    const uint32_t key = r->eax & 0xffu;
    const uint8_t* bytes = GP4_ARRAY(const uint8_t, 0x007c2a48);

    uint32_t index = 0;
    while (index != 22u && key != (bytes[index] & 0x3fu)) {
        ++index;
    }
    if (index == 22u) {
        index = 0;
    }

    const uint32_t result = index >> 1;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result) | (index & 1u));
}
GP4_IMPL(0x005484f2, FUN_005484f2, "regs(eax:u32[0..0x3f]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv22_0052d402")
