#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original loads the dword at +0xc of the 0x1c-byte sample record (base 0x00b1c8e8 + sample * 0x1c)
// and overwrites it at once; the read is kept so the same record is touched. The base arrives as an
// argument so it stays a separate operand of the caller.
static __declspec(noinline) void readSampleWord(uint32_t* slot, uint8_t* base, uint32_t index) {
    uint8_t* record = base + index * 0x1cu;
    *slot = GP4_FIELD(uint32_t, record, 0xc);
}

// Sound back-end hook with a re-entry guard (byte [0x0088fbb5], 0xff while it runs). Unless the guard is
// set, it looks up the sample index of channel eax in [0x0088f948 + 4 * eax], multiplies edx (signed) by
// 0x5622 (imul, then shrd eax, edx, 12: the low 32 bits of the 64-bit product shifted right by 12) and
// passes (sample, scaled) to the empty stdcall stub 0x00509ec0. eax is pushed and popped, so it comes back
// unchanged; edx ends up as the sample index (`pop edx`). The flags are those of the
// `cmp byte [0x0088fbb5], 0xff` on the guarded path, else of the shrd.
// FUNCTION: GP4 0x00518cbe
void __cdecl FUN_00518cbe(gp4::Regs* r) {
    const uint32_t guard = GP4_GLOBAL(uint8_t, 0x0088fbb5);
    if (guard == 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(guard, 0xffu));
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0088fbb5) = 0xffu;

    const uint32_t sample = GP4_ARRAY(uint32_t, 0x0088f948)[r->eax];
    uint32_t unused;
    readSampleWord(&unused, GP4_ARRAY(uint8_t, 0x00b1c8e8), sample);

    const uint64_t bits = static_cast<uint64_t>(static_cast<int64_t>(0x5622) * static_cast<int32_t>(r->edx));
    const uint32_t scaled = static_cast<uint32_t>(bits >> 12);
    gp4::stdcall_<0x00509ec0, void>(sample, scaled);
    GP4_GLOBAL(uint8_t, 0x0088fbb5) = 0u;

    // SHRD by 12: CF = last bit shifted out, AF = 0, OF = bit31(previous ^ result), SF/ZF/PF from the result.
    const uint32_t previous = static_cast<uint32_t>(bits >> 11);
    uint32_t parity = scaled & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = (previous & 1u) | ((~parity & 1u) << 2) | (scaled == 0u ? 0x40u : 0u) |
                           ((scaled & 0x80000000u) != 0u ? 0x80u : 0u) | (((previous ^ scaled) >> 31) << 11);
    r->edx = sample;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00518cbe, FUN_00518cbe, "regs(eax:u32[0..15], edx:u32) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5")
