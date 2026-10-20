#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry `index` of a two-dword array; kept out of line so the constant indexes 0/1 do not fold into
// absolute addresses (the original addresses every array through [edi*4 + base]).
static __declspec(noinline) uint32_t* slotOf(uint32_t base, uint32_t index) {
    return reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(base + index * 4u));
}

// Sound trigger for the record ebx (bit 1 of its dword +4 asks for it) and the source esi, which owns
// one of the two sample slots 0x88fd04/0x88fd08 (samples 0x12/0x13). An owner keeps its slot (released,
// i.e. zeroed, when [0x88fc78] is 0) and restarts only when byte [0x6f3b79] is 0xff. A new source takes
// the other slot (after [0x88fd00]) when [0x88fc78] is nonzero and its level
//   ((((a * b) >> 14) * [ebx+0x94] / 15000, at least 0x100) -> c; (d * a / c * e) >> 14 -> [0x88fcc8])
// (a/b/d/e = the slot's dwords of 0x88fc98/0x88fcb8/0x88fca8/0x88fccc), scaled by [0x7ad8b8] /
// max([0x6f2c00], [0x7ad8b8]), reaches [0x88fc78] (and, with byte [0x66d7a4] set, [0x88fce4] >=
// [0x88fcdc]). Then [0x88fd00] = slot, [0x88f6cc] = slot + 0x12 and, unless muted (byte [0x852d6a]),
// a loaded sample (0x0051f27f CF) gets the request cells [0x88fbac] = 0, [0x88f6e0] = 0xff,
// [0x88f6d0] = 2 * ([0x88f6f0], 0 on a restart), [0x88f6d4] = (c, or the entry ecx on a restart, 0 -> 1)
// << 8, [0x88f6d8] = [0x88fc74] (0xa000 unless byte [0x7ab185] is 0xff) and 0x0051f263 starts it.
// pushal/popal: every register is kept; the flags are those of the last flag writer on the path.
// FUNCTION: GP4 0x00519bb2
void __cdecl FUN_00519bb2(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    const uint32_t source = r->esi;

    const uint32_t request = GP4_FIELD(uint32_t, record, 4) & 2u;
    if (request == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(request));
        return;
    }

    uint32_t slot = 0u;
    uint32_t ecx = r->ecx;
    uint32_t ebp = r->ebp;
    bool owner = true;
    if (source != *slotOf(0x0088fd04u, 0u)) {
        slot = 1u;
        if (source != *slotOf(0x0088fd04u, 1u)) {
            owner = false;
        }
    }

    if (owner) {
        const uint32_t level = GP4_GLOBAL(uint32_t, 0x0088fc78);
        if (level == 0u) {
            *slotOf(0x0088fd04u, slot) = 0u;
        }
        const uint32_t restart = GP4_GLOBAL(uint8_t, 0x006f3b79);
        if (restart != 0xffu) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(restart, 0xffu));
            return;
        }
    } else {
        const uint32_t level = GP4_GLOBAL(uint32_t, 0x0088fc78);
        if (level == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(level));
            return;
        }
        int32_t next = GP4_GLOBAL(int32_t, 0x0088fd00) + 1;
        if (next > 1) {
            next = 0;
        }
        slot = static_cast<uint32_t>(next);

        const uint32_t a = *slotOf(0x0088fc98u, slot);
        const int64_t ab = static_cast<int64_t>(static_cast<int32_t>(a))
                         * static_cast<int32_t>(*slotOf(0x0088fcb8u, slot));
        const uint32_t scaled = static_cast<uint32_t>(ab >> 14);
        uint32_t rem;
        const uint32_t q = gp4::div64(static_cast<uint64_t>(scaled) * GP4_FIELD(uint32_t, record, 0x94), 15000u, &rem);
        ecx = q;
        if (static_cast<int32_t>(ecx) < 0x100) {
            ecx = 0x100u;
        }
        const uint32_t ratio =
            gp4::div64(static_cast<uint64_t>(*slotOf(0x0088fca8u, slot)) * *slotOf(0x0088fc98u, slot), ecx, &rem);
        const int64_t de = static_cast<int64_t>(static_cast<int32_t>(ratio))
                         * static_cast<int32_t>(*slotOf(0x0088fcccu, slot));
        const uint32_t volume = static_cast<uint32_t>(de >> 14);
        GP4_GLOBAL(uint32_t, 0x0088fcc8) = volume;

        const int64_t weighted = static_cast<int64_t>(static_cast<int32_t>(volume))
                               * GP4_GLOBAL(int32_t, 0x007ad8b8);
        int32_t divisor = GP4_GLOBAL(int32_t, 0x006f2c00);
        if (divisor < GP4_GLOBAL(int32_t, 0x007ad8b8)) {
            divisor = GP4_GLOBAL(int32_t, 0x007ad8b8);
        }
        if (divisor == 0) {
            divisor = 1;
        }
        ebp = static_cast<uint32_t>(divisor);
        int32_t irem;
        const int32_t loud = gp4::idiv64(weighted, divisor, &irem);
        if (loud < GP4_GLOBAL(int32_t, 0x0088fc78)) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(loud), level));
            return;
        }
        if (GP4_GLOBAL(uint8_t, 0x0066d7a4) != 0) {
            const uint32_t have = GP4_GLOBAL(uint32_t, 0x0088fce4);
            const uint32_t need = GP4_GLOBAL(uint32_t, 0x0088fcdc);
            if (static_cast<int32_t>(have) < static_cast<int32_t>(need)) {
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(have, need));
                return;
            }
        }
        *slotOf(0x0088fd04u, slot) = source;
    }

    GP4_GLOBAL(uint32_t, 0x0088fd00) = slot;
    const uint32_t sample = slot + 0x12u;
    GP4_GLOBAL(uint32_t, 0x0088f6cc) = sample;
    if (ecx == 0u) {
        ecx = 1u;
    }
    ecx <<= 8;
    uint32_t edx = GP4_GLOBAL(uint32_t, 0x0088f6f0);
    if (GP4_GLOBAL(uint8_t, 0x006f3b79) == 0xffu) {
        edx = 0u;
    }
    const uint32_t muted = GP4_GLOBAL(uint8_t, 0x00852d6a);
    if (muted != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(muted));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x0088f6cc);
    c.ecx = ecx;
    c.edx = edx;
    c.edi = slot;
    c.ebp = ebp;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0xffu, 0u));
    gp4::call_regs(0x0051f27f, c);
    if ((c.eflags & gp4::CF) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0088fbac) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088f6e0) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x0088f6d0) = edx << 1;
    GP4_GLOBAL(uint32_t, 0x0088f6d4) = ecx;
    const uint32_t selector = GP4_GLOBAL(uint8_t, 0x007ab185);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(selector, 0xffu));
    c.eax = (selector == 0xffu) ? GP4_GLOBAL(uint32_t, 0x0088fc74) : 0xa000u;
    GP4_GLOBAL(uint32_t, 0x0088f6d8) = c.eax;
    gp4::call_regs(0x0051f263, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00519bb2, FUN_00519bb2, "regs(ebx:ptr[0x9c]:layout=car_00519bb2, ecx:u32{0,0,0x100,0x12345678,0xffffffff}, esi:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5+chan_00518654+gate_0051ae76+full_00518b09+snd_00518758+snd_0051ace6+eng_00519bb2")
