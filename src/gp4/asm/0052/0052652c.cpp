#include <gp4/gp4.h>
#include <gp4/flags.h>

// Adds a record's three signed words (+6, +8, +0xa) to the heading angles. ebp becomes -1 when |word +6| >=
// 0x4000, else 0. The 16-bit sum word +6 + [0x00679b90] is stored back to 0x00679b90 and 0x00679b5c; the words
// 0x00679b5e and 0x00891510 are negated (16-bit) first when ebp is -1, then get word +8 / +0xa added. The
// sine/cosine routine 0x0052728c then refreshes its tables from the three angles and leaves eax, edx and the
// flags; ebp keeps the -1/0 marker.
// FUNCTION: GP4 0x0052652c
void __cdecl FUN_0052652c(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    uint32_t marker = 0u;
    int32_t magnitude = GP4_FIELD(int16_t, record, 6);
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    if (magnitude >= 0x4000) {
        marker = 0xffffffffu;
    }

    const uint16_t heading = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 6) + GP4_GLOBAL(uint16_t, 0x00679b90));
    GP4_GLOBAL(uint16_t, 0x00679b90) = heading;
    GP4_GLOBAL(uint16_t, 0x00679b5c) = heading;

    uint16_t second = GP4_GLOBAL(uint16_t, 0x00679b5e);
    if (static_cast<int32_t>(marker) < 0) {
        second = static_cast<uint16_t>(0u - second);
    }
    second = static_cast<uint16_t>(second + GP4_FIELD(uint16_t, record, 8));
    GP4_GLOBAL(uint16_t, 0x00679b5e) = second;

    uint16_t third = GP4_GLOBAL(uint16_t, 0x00891510);
    if (static_cast<int32_t>(marker) < 0) {
        third = static_cast<uint16_t>(0u - third);
    }
    third = static_cast<uint16_t>(third + GP4_FIELD(uint16_t, record, 0x0a));
    GP4_GLOBAL(uint16_t, 0x00891510) = third;

    gp4::Regs c = *r;
    c.ebp = marker;
    gp4::call_regs(0x0052728c, c);

    r->eax = c.eax;
    r->edx = c.edx;
    r->ebp = marker;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052652c, FUN_0052652c, "regs(ebx:ptr[0x10]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
