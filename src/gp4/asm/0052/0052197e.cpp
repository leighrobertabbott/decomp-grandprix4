#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0052197e
// Looks up ebx in the pointer table at 0x893eb0. When the entry is the shared default 0x8940f8,
// the mode index from 0x0051fb4f selects instead the entry [0x894054 + 4*index], or, when a car
// is selected in [0x679d2c] and its byte +0x7f is neither 0 nor 1, the entry [0x89406c + 4*index].
// eax and esi are saved and restored; ebx returns the pointer. The flags are those of the last
// compare that ran (cmp ebx / or esi / cmp byte +0x7f).
void __cdecl FUN_0052197e(gp4::Regs* r) {
    uint32_t pointer = GP4_ARRAY(uint32_t, 0x00893eb0)[r->ebx];
    uint32_t flags = gp4::flags::sub32(pointer, 0x008940f8u);

    if (pointer == 0x008940f8u) {
        gp4::Regs mode = {};
        gp4::call_regs(0x0051fb4f, mode);
        const uint32_t index = mode.eax;

        pointer = GP4_ARRAY(uint32_t, 0x00894054)[index];
        const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d2c);
        flags = gp4::flags::logic32(car);
        if (car != 0u) {
            const uint32_t kind = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(car + 0x7fu));
            flags = gp4::flags::sub8(kind, 0u);
            if (kind != 0u) {
                flags = gp4::flags::sub8(kind, 1u);
                if (kind != 1u) {
                    pointer = GP4_ARRAY(uint32_t, 0x0089406c)[index];
                }
            }
        }
    }

    r->ebx = pointer;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052197e, FUN_0052197e, "regs(ebx:u32[0..9]) -> (ebx, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_004059ef+player_0040f8d9+car_0052197e")
