#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call target so the compiler keeps three distinct call sites.
static __declspec(noinline) void CallIsqrt(gp4::Regs& c) {
    gp4::call_regs(0x004010f3, c);
}
static __declspec(noinline) void CallSampleLoaded(gp4::Regs& c) {
    gp4::call_regs(0x0051f27f, c);
}
static __declspec(noinline) void CallSampleStart(gp4::Regs& c) {
    gp4::call_regs(0x0051f263, c);
}

// (d * d) >> 14 of the signed 64-bit product, low 32 bits (imul eax; shrd eax, edx, 0xe).
static inline uint32_t SquareQ14(int32_t d) {
    return static_cast<uint32_t>((static_cast<int64_t>(d) * d) >> 14);
}

// pushal wrapper over the 16 entries of 0x0063decc (stride 0x2c). For each entry with bit 0 of
// dword +0 set: the squared distance of its position (+0xc/+0x10/+0x14, >> 5) from the point
// 0x7a8dd4/d8/dc goes through the integer square root 0x4010f3 (clamped to >= 0x80); the level +0x28
// << 7 is divided by it (clamped to <= 0x4000) and scaled by [0x88f6f0] >> 14. When the pause byte
// [0x852d6a] is zero, sample 0xc is stored in [0x88f6cc] and 0x51f27f tests whether it is loaded
// (CF): if so [0x88fbac] = 0, [0x88f6e0] = 0xff, [0x88f6d0] = value * 2, [0x88f6d4] = 0x100000,
// [0x88f6d8] = 0xa000 and 0x51f263 starts it. popal restores every register; the flags are those
// of the final `dec ecx` after `add ebx, 0x2c`.
// FUNCTION: GP4 0x0051a87e
void __cdecl FUN_0051a87e(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t rec = reinterpret_cast<uint32_t>(gp4::cdecl_<0x00418914, uint8_t*>());
    uint32_t counter = 0x10u;
    c.ebx = rec;
    c.ecx = counter;
    uint32_t flags;
    do {
        const uint32_t* entry = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(rec));
        if ((entry[0] & 1u) != 0u) {
            const int32_t dx = static_cast<int32_t>(
                static_cast<uint32_t>(static_cast<int32_t>(entry[3]) >> 5) - GP4_GLOBAL(uint32_t, 0x007a8dd4));
            const int32_t dy = static_cast<int32_t>(
                static_cast<uint32_t>(static_cast<int32_t>(entry[4]) >> 5) - GP4_GLOBAL(uint32_t, 0x007a8dd8));
            const int32_t dz = static_cast<int32_t>(
                static_cast<uint32_t>(static_cast<int32_t>(entry[5]) >> 5) - GP4_GLOBAL(uint32_t, 0x007a8ddc));
            const uint32_t partial = SquareQ14(dx) + SquareQ14(dy);
            const int64_t zz = static_cast<int64_t>(dz) * dz;
            c.ebp = partial;
            c.edx = static_cast<uint32_t>(static_cast<uint64_t>(zz) >> 32);
            c.eax = static_cast<uint32_t>(zz >> 14) + partial;
            CallIsqrt(c);
            int32_t divisor = static_cast<int32_t>(c.eax);
            if (divisor < 0x80) {
                divisor = 0x80;
            }
            c.ebp = static_cast<uint32_t>(divisor);
            int32_t remainder;
            int32_t level = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(entry[10])) << 7, divisor, &remainder);
            if (level > 0x4000) {
                level = 0x4000;
            }
            const uint32_t value = static_cast<uint32_t>(
                (static_cast<int64_t>(level) * GP4_GLOBAL(int32_t, 0x0088f6f0)) >> 14);
            c.edx = value;
            if (GP4_GLOBAL(uint8_t, 0x00852d6a) == 0u) {
                c.eax = 0xcu;
                GP4_GLOBAL(uint32_t, 0x0088f6cc) = 0xcu;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0xffu, 0u));
                CallSampleLoaded(c);
                if ((c.eflags & gp4::flags::CF) != 0u) {
                    GP4_GLOBAL(uint32_t, 0x0088fbac) = 0u;
                    GP4_GLOBAL(uint8_t, 0x0088f6e0) = 0xffu;
                    GP4_GLOBAL(uint32_t, 0x0088f6d0) = value << 1;
                    GP4_GLOBAL(uint32_t, 0x0088f6d4) = 0x100000u;
                    GP4_GLOBAL(uint32_t, 0x0088f6d8) = 0xa000u;
                    c.eax = 0xa000u;
                    c.eflags = gp4::flags::merge(
                        c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007ab185), 0xffu));
                    CallSampleStart(c);
                }
            }
            c.eax = value;
        }
        flags = gp4::flags::dec32(counter, gp4::flags::add32(rec, 0x2cu));
        rec += 0x2cu;
        counter -= 1u;
        c.ebx = rec;
        c.ecx = counter;
    } while (counter != 0u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051a87e, FUN_0051a87e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+chan_005188f5+chan_00518654+snd_00518758+sndsel_00518545+sndrec_0051a78d+sndgate_0051a78d")
