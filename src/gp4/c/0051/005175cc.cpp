#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dword at 4 * index + base (a helper so the table base 0x891bd0 stays a global).
static __declspec(noinline) uint32_t TableAt(const uint32_t* base, uint32_t index) {
    return base[index];
}

static inline void SetAl(gp4::Regs& c, uint32_t value) {
    c.eax = (c.eax & 0xffffff00u) | (value & 0xffu);
}

// The text-stream glyph map ('-' -> 0x15, digits '0'..'9' -> 0x16..0x1f); applied here to the
// literal space, which it leaves unchanged.
static inline uint8_t MapGlyph(uint8_t ch) {
    if (ch == 0x2du) {
        ch = 0x15u;
    }
    if (ch >= 0x30u && ch <= 0x39u) {
        ch = static_cast<uint8_t>(ch - 0x1au);
    }
    return ch;
}

// pushal/popal script: claims id 8 in the bitmap through 0x51d187, sets the state bytes
// 0x88fd9f = 2, 0x88fda0 = 0x49 and 0x88fd94..97 = 3, 1, 0, 2, [0xa4b34c] = 1, then 0x88fda0 =
// 0xa7, emits the 5-byte header (0x516927), takes the dword key of the first order byte
// ([0x675970 + 4 * (([0x675958] - 1) & 0x3f)], remembered in [0xa4b350]) and writes it as a time
// through 0x5143f5 with the stream flag 0x89152b set, appends a space, the bytes 2 and 0x88fda0
// (0x27), the driver name of that car number (0x51f2af -> [0xa4b354], copied by 0x516d94) and runs
// the closing empty/reset routines. Every register is preserved; the flags are those the name copy
// (0x516d94) left.
// FUNCTION: GP4 0x005175cc
void __cdecl FUN_005175cc(gp4::Regs* r) {
    gp4::Regs c = *r;

    c.eax = 8u;
    gp4::call_regs(0x0051d187, c);
    SetAl(c, 2u);
    gp4::call_regs(0x0051d494, c);
    SetAl(c, 0x49u);
    gp4::call_regs(0x0051d48e, c);

    GP4_GLOBAL(uint8_t, 0x0088fd94) = 3;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 1;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 2;
    GP4_GLOBAL(uint32_t, 0x00a4b34c) = 1u;

    c.esi = TableAt(GP4_ARRAY(uint32_t, 0x00891bd0), 0x2cu);
    SetAl(c, 0xa7u);
    gp4::call_regs(0x0051d48e, c);
    gp4::call_regs(0x00516927, c);
    gp4::cdecl_<0x0051734d, void>();

    const uint32_t index = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00675958) - 1u) & 0x3fu;
    uint32_t* key = GP4_ARRAY(uint32_t, 0x00675970) + index;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(key));
    c.eax = *key;
    GP4_GLOBAL(uint32_t, 0x00a4b350) = c.eax;

    GP4_GLOBAL(uint8_t, 0x0089152b) = 1;
    gp4::call_regs(0x005143f5, c);
    GP4_GLOBAL(uint8_t, 0x0089152b) = 0;

    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    out[0] = MapGlyph(0x20u);
    SetAl(c, 0x27u);
    gp4::call_regs(0x0051d48e, c);
    out[1] = 2;
    out[2] = GP4_GLOBAL(uint8_t, 0x0088fda0);
    c.edi += 3u;

    SetAl(c, GP4_GLOBAL(uint8_t, 0x00675958));
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a4b354) = c.eax;
    c.esi = c.eax;
    gp4::call_regs(0x00516d94, c);
    const uint32_t nameFlags = c.eflags;

    gp4::cdecl_<0x00516940, void>();
    gp4::call_regs(0x005142b4, c);

    r->eflags = gp4::flags::merge(r->eflags, nameFlags);
}
GP4_IMPL(0x005175cc, FUN_005175cc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_00675970+bits_005174cb")
