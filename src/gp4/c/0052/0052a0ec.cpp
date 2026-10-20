#include <gp4/gp4.h>
#include <gp4/flags.h>

// The two arms read different globals; MSVC would turn the select into base + delta arithmetic and hide
// one of the two addresses, so each arm gets its own out-of-line read.
static __declspec(noinline) uint32_t EntryFromSaved() {
    return GP4_GLOBAL(uint32_t, 0x0081d1dc);
}

static __declspec(noinline) uint32_t EntryFromCircuit() {
    return GP4_GLOBAL(uint32_t, 0x007ad894);
}

// Stores the four bytes at 0x0081d1d4 into the 0xe4-byte entry of the record array at 0x007adb4c that
// 0x0052a260 selects: record = [0x007c2f28], entry = [0x0081d1dc] when the byte [0x0081d1e8] is 0xff, else
// [0x007ad894] (the current circuit). The array walker 0x0052ab04 runs before and after; the middle part is
// a pushal/popal block, so the registers are unchanged and the flags are those of the closing
// 0x0052ab04.
// FUNCTION: GP4 0x0052a0ec
void __cdecl FUN_0052a0ec(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052ab04, c);

    gp4::Regs d = c;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x007c2f28);
    uint32_t entry;
    d.eflags = gp4::flags::merge(d.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081d1e8), 0xffu));
    if (GP4_GLOBAL(uint8_t, 0x0081d1e8) == 0xffu) {
        entry = EntryFromSaved();
    } else {
        entry = EntryFromCircuit();
    }
    d.eax = record;
    d.ebx = entry;
    gp4::call_regs(0x0052a260, d);

    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(d.eax));
    const uint8_t* source = GP4_ARRAY(const uint8_t, 0x0081d1d4);
    for (uint32_t i = 0u; i != 4u; ++i) {
        target[i] = source[i];
    }

    c.eflags = d.eflags;
    gp4::call_regs(0x0052ab04, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052a0ec, FUN_0052a0ec, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+ent_0052a0ec")
