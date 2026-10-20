#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call target so the compiler keeps three distinct call sites.
static __declspec(noinline) void CallRecordEntry(gp4::Regs& c) {
    gp4::call_regs(0x00418901, c);
}
static __declspec(noinline) void CallSampleLoaded(gp4::Regs& c) {
    gp4::call_regs(0x0051f27f, c);
}
static __declspec(noinline) void CallSampleStart(gp4::Regs& c) {
    gp4::call_regs(0x0051f263, c);
}

// Walks the 22 car records of 0x0066fef0 (esi, stride 0x41c, counter ecx). For each, 0x418901 gives
// the record's telemetry entry (eax, kept in ebx); when its flag dword has bit 0x400 and the pause
// byte [0x852d6a] is zero, sample 0xb is stored in [0x88f6cc] and 0x51f27f asks whether it is loaded
// (CF set): if so [0x88fbac] = 0, the channel byte [0x88f6e0] = 0xff, the volume fields
// [0x88f6d0] = [0x88f6f0] * 2, [0x88f6d4] = 0x100000, [0x88f6d8] = 0xa000 are set and 0x51f263
// starts it. eax is the last entry address on exit (every path restores it); the flags are those
// of the final `dec ecx` after `add esi, 0x41c`.
// FUNCTION: GP4 0x0051a7e0
void __cdecl FUN_0051a7e0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = 0x16u;
    c.esi = 0x0066fef0u;
    uint32_t flags;
    do {
        const uint32_t counter = c.ecx;
        CallRecordEntry(c);
        c.ebx = c.eax;
        const uint32_t entry = c.eax;
        if ((*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry)) & 0x400u) != 0u) {
            if (GP4_GLOBAL(uint8_t, 0x00852d6a) == 0u) {
                c.eax = 0xbu;
                GP4_GLOBAL(uint32_t, 0x0088f6cc) = 0xbu;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0xffu, 0u));
                CallSampleLoaded(c);
                if ((c.eflags & gp4::flags::CF) != 0u) {
                    GP4_GLOBAL(uint32_t, 0x0088fbac) = 0u;
                    GP4_GLOBAL(uint8_t, 0x0088f6e0) = 0xffu;
                    GP4_GLOBAL(uint32_t, 0x0088f6d0) = GP4_GLOBAL(uint32_t, 0x0088f6f0) << 1;
                    GP4_GLOBAL(uint32_t, 0x0088f6d4) = 0x100000u;
                    GP4_GLOBAL(uint32_t, 0x0088f6d8) = 0xa000u;
                    c.eflags = gp4::flags::merge(
                        c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007ab185), 0xffu));
                    CallSampleStart(c);
                }
                c.eax = entry;
            }
        }
        c.ecx = counter;
        c.esi += 0x41cu;
        flags = gp4::flags::dec32(c.ecx, gp4::flags::add32(c.esi - 0x41cu, 0x41cu));
        c.ecx -= 1u;
        c.eflags = gp4::flags::merge(c.eflags, flags);
    } while (c.ecx != 0u);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051a7e0, FUN_0051a7e0, "regs() -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+chan_005188f5+chan_00518654+snd_00518758+state_00418d27+sndgate_0051a78d")
