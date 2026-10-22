#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original loads edi with the list block 0x00830b00 and reaches its fields through it; keep the base
// visible instead of letting the compiler turn the fields into extra globals.
static __declspec(noinline) uint32_t* ListBlock() {
    return reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x00830b00u));
}

// One out-of-line call per arm of the sort selector (the arms differ only in the callee address).
static __declspec(noinline) void SortByClass(gp4::Regs& c) {
    gp4::call_regs(0x00542257, c);
}

static __declspec(noinline) void SortByKeyDescending(gp4::Regs& c) {
    gp4::call_regs(0x005421e5, c);
}

static __declspec(noinline) void SortByKeyAscending(gp4::Regs& c) {
    gp4::call_regs(0x00542224, c);
}

// Builds the pointer table of the datalog channel list: the count [0x00830bac] is clamped to 0x28 (an
// empty routine 0x0054e232 is called when it is larger) and stored at +4 of the list block 0x00830b00 (the
// cursor at +0 is cleared when the count does not exceed the window at +8); entries 0x00830b0c.. then
// point at the 0x20-byte records from 0x00830bb4. A non-empty table is sorted by the mode byte
// [0x008310b4] (0: 0x00542257, 1: 0x005421e5, anything else: 0x00542224) and 0x00543942 then moves the
// cursor. Exit registers: esi = 0x00830bb4 + 0x20 * count, eax/ecx/edi/flags from 0x00543942.
// FUNCTION: GP4 0x005420a8
void __cdecl FUN_005420a8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    int32_t count = GP4_GLOBAL(int32_t, 0x00830bac);
    uint32_t* block = ListBlock();
    if (count > 0x28) {
        gp4::cdecl_<0x0054e232, void>();
        count = 0x28;
    }
    block[1] = static_cast<uint32_t>(count);
    if (count <= static_cast<int32_t>(block[2])) {
        block[0] = 0u;
    }

    uint32_t record = 0x00830bb4u;
    uint32_t* slot = block + 3;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(static_cast<uint32_t>(count)));
    if (count != 0) {
        for (uint32_t k = static_cast<uint32_t>(count); k != 0u; --k) {
            *slot = record;
            record += 0x20u;
            ++slot;
        }
        const uint32_t selector = GP4_GLOBAL(uint8_t, 0x008310b4);
        const uint32_t which = (selector < 3u) ? selector : 2u;
        c.eax = which * 4u + 0x00542128u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(which * 4u, 0x00542128u));
        c.esi = record;
        c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
        c.ecx = 0u;
        if (which == 0u) {
            SortByClass(c);
        } else if (which == 1u) {
            SortByKeyDescending(c);
        } else {
            SortByKeyAscending(c);
        }
    } else {
        c.esi = record;
        c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
        c.ecx = 0u;
    }
    gp4::call_regs(0x00543942, c);

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005420a8, FUN_005420a8, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sort_00542224+sortkeys_005421e5+recs_005438a0+cur_00543942+dl_005420a8")
