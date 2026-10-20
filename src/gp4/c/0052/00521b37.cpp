#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps a (index eax, value edx) pair against a per-index range table. The table record comes from
// 0x0052197e (ebx = the index [0x0066d8d8] on the way in, the record pointer on the way out). The
// record is { byte ?, byte count n, n bytes, n bytes, n lows, n highs }: eax is clamped to
// 0..n-1 (0 when n is 0), then edx is raised to the low byte lows[eax] and lowered to the high byte
// highs[eax] (both signed compares). ebp, ebx and ecx are saved and restored; the flags are those of
// the final `cmp edx, high`.

// FUNCTION: GP4 0x00521b37
void __cdecl FUN_00521b37(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebx = GP4_GLOBAL(uint32_t, 0x0066d8d8);
    gp4::call_regs(0x0052197e, c);

    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.ebx));
    const uint32_t count = record[1];
    int32_t last = static_cast<int32_t>(count) - 1;
    if (last < 0) {
        last = 0;
    }
    int32_t index = static_cast<int32_t>(c.eax);
    if (index > last) {
        index = last;
    }
    if (index < 0) {
        index = 0;
    }

    const uint32_t low = record[2u + 2u * count + static_cast<uint32_t>(index)];
    int32_t value = static_cast<int32_t>(c.edx);
    if (value < static_cast<int32_t>(low)) {
        value = static_cast<int32_t>(low);
    }
    const uint32_t high = record[2u + 3u * count + static_cast<uint32_t>(index)];
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(value), high);
    if (value > static_cast<int32_t>(high)) {
        value = static_cast<int32_t>(high);
    }

    r->eax = static_cast<uint32_t>(index);
    r->edx = static_cast<uint32_t>(value);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00521b37, FUN_00521b37, "regs(eax:i32, edx:i32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_004059ef+player_0040f8d9+car_0052197e+links_00521d6f")
