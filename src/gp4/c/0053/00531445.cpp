#include <gp4/gp4.h>
#include <gp4/flags.h>

// Piecewise-linear lookup of a speed-dependent value. The rate [0x00833c98] gives the key
// 0x4c9a988 / rate (0 when the rate is 0), clamped to 0x10a..1000. The table of 24-byte rows
// (key dword, then five value dwords) is 0x005314da when the dword at [ebx] is zero, else 0x00531582; the
// row whose next key is not below the key is used, the interpolation parameter is
// ((key - row key) << 14) / (next key - row key) and the result edx is
// row[ecx] + (((next[ecx] - row[ecx]) * parameter + 0x2000) >> 14) with the 64-bit shift keeping only the
// low 32 bits (the +0x2000 rounding is added to the low dword alone). eax, ebx, ecx, esi and edi are
// restored; the flags are those of the final `add edx, eax`.
// FUNCTION: GP4 0x00531445
void __cdecl FUN_00531445(gp4::Regs* r) {
    const uint32_t selector = r->ebx;
    const uint32_t column = r->ecx;

    const uint32_t rate = GP4_GLOBAL(uint32_t, 0x00833c98);
    uint32_t value = rate;
    if (rate != 0u) {
        value = 0x04c9a988u / rate;
    }
    int32_t key = static_cast<int32_t>(value);
    if (key < 0x10a) {
        key = 0x10a;
    }
    if (key > 0x3e8) {
        key = 0x3e8;
    }

    const int32_t* row = reinterpret_cast<const int32_t*>(
        static_cast<uintptr_t>(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(selector)) == 0u ? 0x005314dau : 0x00531582u));
    while (key > row[6]) {
        row += 6;
    }

    const int32_t offset = key - row[0];
    const int32_t span = row[6] - row[0];
    int32_t remainder;
    const int32_t parameter = gp4::idiv64(static_cast<int64_t>(offset) << 14, span, &remainder);

    const uint32_t base = static_cast<uint32_t>(row[1 + column]);
    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(row[7 + column]) - base);
    const int64_t product = static_cast<int64_t>(delta) * static_cast<int64_t>(parameter);
    const uint32_t low = static_cast<uint32_t>(product) + 0x2000u;
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t scaled = (low >> 14) | (high << 18);

    r->edx = base + scaled;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(base, scaled));
}
GP4_IMPL(0x00531445, FUN_00531445, "regs(ebx:ptr[8]:u32{=0,1,2,0x80000000,0xffffffff}, ecx:u32[0..4]) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
