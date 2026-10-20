#include <gp4/gp4.h>
#include <gp4/flags.h>

// Index of the 0x1a-entry stream-pointer table that starts at 0x0083878e (0x835846 + 0xbd2 * 4).
static __declspec(noinline) uint32_t TableAt(uint32_t index) {
    return (index << 2) + 0x00835846u;
}

// Fills a table of 0x1a pointers: the first ecx entries (ecx = 1..0x1a) each get the current byte-code
// stream position esi, which is then advanced past one zero-terminated entry with the skipper 0x0054d0b9;
// the remaining 0x1a - ecx entries get the address 0x00835807 (the empty entry). Exit: edi past the table,
// ecx = 0, esi past the skipped entries. The flags are those of `add ecx, 0x1a` when the table was filled
// by the stream alone, else of the last `add edi, 4`.
// FUNCTION: GP4 0x0054cfbd
void __cdecl FUN_0054cfbd(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    uint32_t edi = TableAt(0xbd2u);
    gp4::Regs c = *r;

    uint32_t left = count;
    do {
        GP4_FIELD(uint32_t, edi, 0) = c.esi;
        edi += 4u;
        c.edi = edi;
        c.ecx = left;
        gp4::call_regs(0x0054d0b9, c);
        --left;
    } while (left != 0u);

    uint32_t remaining = 0x1au - count;
    uint32_t arith = gp4::flags::add32(0u - count, 0x1au);
    if (remaining != 0u) {
        do {
            GP4_FIELD(uint32_t, edi, 0) = 0x00835807u;
            arith = gp4::flags::add32(edi, 4u);
            edi += 4u;
            --remaining;
        } while (remaining != 0u);
    }

    c.edi = edi;
    c.ecx = 0u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    *r = c;
}
GP4_IMPL(0x0054cfbd, FUN_0054cfbd, "regs(ecx:u32[1..26], esi:ptr[0x800]:u8{=0,0,0x14,0x20,0x41,0xff}) -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
