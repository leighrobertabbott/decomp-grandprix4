#include <gp4/gp4.h>
#include <gp4/flags.h>

// Date formatter into the text buffer 0x00823100 (the cursor edi). eax is a day count: 0x0052b02b turns
// it into year (ecx), month (dh) and day (dl), which are stored at 0x0081d368 / 0x0081d36d / 0x0081d36c.
// The byte 0x007ab17c selects the field order (0: month-day-year, 1: day-month-year, anything >= 2:
// year-month-day) and the byte 0x0082974e the style: 0xff writes the three fields as plain numbers
// separated by the character 0x00829750, anything else writes the month name, the day text and the
// zero-padded year separated by spaces. The buffer is NUL-terminated at the end. Every register is
// threaded through the callees (eax holds the jump-table address when the first callee runs); the
// flags handed to the first callee are those of `cmp byte [0x82974e], 0xff`, and the final registers
// and flags are what the last callee leaves.
static __declspec(noinline) void NumericMonthDayYear(gp4::Regs& c) {
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36d);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36c);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d368);
    gp4::call_regs(0x0054dac1, c);
}

static __declspec(noinline) void NumericDayMonthYear(gp4::Regs& c) {
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36c);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36d);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d368);
    gp4::call_regs(0x0054dac1, c);
}

static __declspec(noinline) void NumericYearMonthDay(gp4::Regs& c) {
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d368);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36d);
    gp4::call_regs(0x0054dac1, c);
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054daad, c);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36c);
    gp4::call_regs(0x0054dac1, c);
}

static __declspec(noinline) void TextMonthDayYear(gp4::Regs& c) {
    gp4::call_regs(0x0052afce, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afb7, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afec, c);
}

static __declspec(noinline) void TextDayMonthYear(gp4::Regs& c) {
    gp4::call_regs(0x0052afb7, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afce, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afec, c);
}

static __declspec(noinline) void TextYearMonthDay(gp4::Regs& c) {
    gp4::call_regs(0x0052afec, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afce, c);
    gp4::call_regs(0x0054da9d, c);
    gp4::call_regs(0x0052afb7, c);
}

// FUNCTION: GP4 0x0052ae53
void __cdecl FUN_0052ae53(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823100u;
    gp4::call_regs(0x0052b02b, c);

    GP4_GLOBAL(uint8_t, 0x0081d36c) = static_cast<uint8_t>(c.edx);
    GP4_GLOBAL(uint8_t, 0x0081d36d) = static_cast<uint8_t>(c.edx >> 8);
    GP4_GLOBAL(uint32_t, 0x0081d368) = c.ecx;

    uint32_t order = GP4_GLOBAL(uint8_t, 0x007ab17c);
    if (order >= 3u) {
        order = 2u;
    }
    c.eax = 0x0052ae8cu + order * 4u;

    const uint32_t cmp = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0082974e), 0xffu);
    c.eflags = gp4::flags::merge(c.eflags, cmp);
    const bool numeric = GP4_GLOBAL(uint8_t, 0x0082974e) == 0xffu;

    switch (order) {
    case 0:
        if (numeric) {
            NumericMonthDayYear(c);
        } else {
            TextMonthDayYear(c);
        }
        break;
    case 1:
        if (numeric) {
            NumericDayMonthYear(c);
        } else {
            TextDayMonthYear(c);
        }
        break;
    default:
        if (numeric) {
            NumericYearMonthDay(c);
        } else {
            TextYearMonthDay(c);
        }
        break;
    }

    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = 0;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052ae53, FUN_0052ae53, "regs(eax:u32[0..0x20000], edx:u32, esi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98+script_00545c84")
