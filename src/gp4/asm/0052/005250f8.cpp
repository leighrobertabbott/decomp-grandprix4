#include <gp4/gp4.h>
#include <gp4/flags.h>

// Distance along the lap between the selected car's position word (+0x1c of the record at
// [0x00679d30]) and table entry ebx of [0x006f2f6c]. Both positions must lie below the lap limit
// [0x00891500], otherwise CF is set and eax/edx hold the raw values. Otherwise the absolute
// difference is taken and, when it reaches the half-lap threshold [0x00866ed8] (signed), is
// replaced by [0x00866ec4] - |difference| (the way round the other side of the lap); the result
// goes to eax and ecx and CF is cleared (the other flags are those of the last flag writer).
// FUNCTION: GP4 0x005250f8
void __cdecl FUN_005250f8(gp4::Regs* r) {
    const uint32_t table_value = GP4_ARRAY(uint32_t, 0x006f2f6c)[r->ebx];
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t eax = GP4_FIELD(uint32_t, (uint8_t*)car, 0x1c);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);

    r->edx = table_value;
    uint32_t f = gp4::flags::sub32(table_value, limit);
    if (table_value >= limit) {
        r->eax = eax;
        r->eflags = gp4::flags::merge(r->eflags, f | gp4::flags::CF);
        return;
    }
    f = gp4::flags::sub32(eax, limit);
    if (eax >= limit) {
        r->eax = eax;
        r->eflags = gp4::flags::merge(r->eflags, f | gp4::flags::CF);
        return;
    }

    eax -= table_value;
    uint32_t ecx = eax;
    if (static_cast<int32_t>(ecx) < 0) {
        ecx = 0u - ecx;
    }
    const uint32_t half = GP4_GLOBAL(uint32_t, 0x00866ed8);
    f = gp4::flags::sub32(ecx, half);
    if (static_cast<int32_t>(ecx) >= static_cast<int32_t>(half)) {
        ecx = 0u - ecx;
        ecx += GP4_GLOBAL(uint32_t, 0x00866ec4);
        f = gp4::flags::neg32(eax);
    }

    r->eax = ecx;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, f & ~gp4::flags::CF);
}
GP4_IMPL(0x005250f8, FUN_005250f8, "regs(ebx:u32[0..63]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d")
