#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dword at 4 * index + base (a helper so the table base 0x891bd0 stays a global).
static __declspec(noinline) uint32_t TableAt(const uint32_t* base, uint32_t index) {
    return base[index];
}

static inline void SetAl(gp4::Regs& c, uint32_t value) {
    c.eax = (c.eax & 0xffffff00u) | (value & 0xffu);
}

// pushal/popal script for the text-stream entry of player slot [0x66d818]: claims id 3 in the
// bitmap through 0x51d187, sets the state bytes 0x88fd9f = 2, 0x88fda0 = 0x4f and
// 0x88fd94..97 = 5, 1, 0, 2, looks up the driver name of the car number (byte +0x7c of the car
// record [0x66d818]) through 0x51f2af (remembered in [0xa4b358]), emits the 5-byte header
// (0x516927) and the name (0x516d94, copied to 0x891531...), then appends the bytes 2 and
// 0x88fda0 (0x27 after the callee-set 0x4f) at edi and runs the closing empty/reset routines. Every
// register is preserved; the flags are those the name copy (0x516d94) left.
// FUNCTION: GP4 0x0051754b
void __cdecl FUN_0051754b(gp4::Regs* r) {
    gp4::Regs c = *r;

    c.eax = 3u;
    gp4::call_regs(0x0051d187, c);
    SetAl(c, 2u);
    gp4::call_regs(0x0051d494, c);
    SetAl(c, 0x4fu);
    gp4::call_regs(0x0051d48e, c);

    GP4_GLOBAL(uint8_t, 0x0088fd94) = 5;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 1;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 2;

    c.esi = GP4_GLOBAL(uint32_t, 0x0066d818);
    SetAl(c, GP4_FIELD(uint8_t, c.esi, 0x7c));
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a4b358) = c.eax;
    c.esi = c.eax;
    gp4::call_regs(0x00516927, c);
    gp4::call_regs(0x00516d94, c);
    const uint32_t nameFlags = c.eflags;

    c.esi = TableAt(GP4_ARRAY(uint32_t, 0x00891bd0), 0x2bu);
    gp4::cdecl_<0x0051734d, void>();
    SetAl(c, 0x27u);
    gp4::call_regs(0x0051d48e, c);

    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    out[0] = 2;
    out[1] = GP4_GLOBAL(uint8_t, 0x0088fda0);
    c.edi += 2u;
    gp4::cdecl_<0x00516940, void>();
    gp4::call_regs(0x005142b4, c);

    r->eflags = gp4::flags::merge(r->eflags, nameFlags);
}
GP4_IMPL(0x0051754b, FUN_0051754b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrs_0066d818+carsel_0051754b+carno_0066ff6c")
