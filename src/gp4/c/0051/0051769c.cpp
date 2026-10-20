#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dword at 4 * index + base (a helper so the table base 0x891bd0 stays a global).
static __declspec(noinline) uint32_t TableAt(const uint32_t* base, uint32_t index) {
    return base[index];
}

static inline void SetAl(gp4::Regs& c, uint32_t value) {
    c.eax = (c.eax & 0xffffff00u) | (value & 0xffu);
}

// pushal/popal script for the second text-stream entry (sibling of 0x51754b, other order): claims id 7
// in the bitmap through 0x51d187, sets 0x88fd9f = 2, 0x88fda0 = 0x27 and 0x88fd94..97 = 8, 1, 0, 2,
// emits the 5-byte header (0x516927, esi = table entry 0x29 of 0x891bd0), appends the bytes 2 and
// 0x88fda0 at edi (past the header), looks up the driver name of the car number (byte +0x7c of the
// car record [0x66d824]) through 0x51f2af (remembered in [0xa4b35c]) and copies it (0x516d94) after
// those two bytes. Every register is preserved; the flags are those the name copy left.
// FUNCTION: GP4 0x0051769c
void __cdecl FUN_0051769c(gp4::Regs* r) {
    gp4::Regs c = *r;

    c.eax = 7u;
    gp4::call_regs(0x0051d187, c);
    SetAl(c, 2u);
    gp4::call_regs(0x0051d494, c);
    SetAl(c, 0x27u);
    gp4::call_regs(0x0051d48e, c);

    GP4_GLOBAL(uint8_t, 0x0088fd94) = 8;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 1;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 2;

    c.esi = TableAt(GP4_ARRAY(uint32_t, 0x00891bd0), 0x29u);
    gp4::call_regs(0x00516927, c);
    gp4::cdecl_<0x0051734d, void>();
    SetAl(c, 0x27u);
    gp4::call_regs(0x0051d48e, c);

    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    out[0] = 2;
    out[1] = GP4_GLOBAL(uint8_t, 0x0088fda0);
    c.edi += 2u;

    c.esi = GP4_GLOBAL(uint32_t, 0x0066d824);
    SetAl(c, GP4_FIELD(uint8_t, c.esi, 0x7c));
    gp4::call_regs(0x0051f2af, c);
    GP4_GLOBAL(uint32_t, 0x00a4b35c) = c.eax;
    c.esi = c.eax;
    gp4::call_regs(0x00516d94, c);
    const uint32_t nameFlags = c.eflags;

    gp4::cdecl_<0x00516940, void>();
    gp4::call_regs(0x005142b4, c);

    r->eflags = gp4::flags::merge(r->eflags, nameFlags);
}
GP4_IMPL(0x0051769c, FUN_0051769c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrs_0066d818+carsel_0051754b+carno_0066ff6c")
