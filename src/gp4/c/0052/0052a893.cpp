#include <gp4/gp4.h>
#include <gp4/flags.h>

// Three rows of colour-word stores followed by three frame-cell hand-offs. With the window origin
// [0x0081d1cc] the position eax starts at origin + 0xc (ecx = eax + 0x18) and ebx at [0x0081d1c8] + 0x110
// (ebp = ebx + 0x18). Each of the three rows stores the next word of the table at 0x0052a90d (the code bytes
// right after this routine's ret: lodsd) and 0x6b5a through the colour setter 0x00531af4, runs the empty
// stubs 0x00531b2e / 0x00531b2c, and moves eax and ecx on by 0x2c. Then ebx = [0x0081d1c8] + 0x12c and
// eax = origin + 0x5c, and the frame-cell hand-off 0x0052800e runs for the frame numbers edx = 0x2c3, 0x2c2
// and 0x2c1 with eax stepping back by 0x2c each time. 0x00531af4 and 0x0052800e are pushal/popal routines, so
// the registers are those this code leaves: eax/ebx/ecx/edx/esi/ebp as computed, edi untouched; the flags
// are those of the last `cmp edx, 0x2c0` (equal).

// One helper per empty stub so every call keeps its own address.
static __declspec(noinline) void EmptyStubAfterColour() { gp4::cdecl_<0x00531b2e, void>(); }
static __declspec(noinline) void EmptyStubAfterRow() { gp4::cdecl_<0x00531b2c, void>(); }

// FUNCTION: GP4 0x0052a893
void __cdecl FUN_0052a893(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.ebx = GP4_GLOBAL(uint32_t, 0x0081d1c8) + 0x110u;
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d1cc) + 0xcu;
    c.ebp = c.ebx;
    c.ecx = c.eax;
    c.ebp += 0x18u;
    c.ecx += 0x18u;
    c.esi = 0x0052a90du;
    c.edx = 3u;
    do {
        const uint32_t position = c.eax;
        c.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
        c.esi += 4u;
        gp4::call_regs(0x00531af4, c);
        c.eax = position;
        EmptyStubAfterColour();
        c.eax = 0x6b5au;
        gp4::call_regs(0x00531af4, c);
        c.eax = position;
        EmptyStubAfterRow();
        c.eax += 0x2cu;
        c.ecx += 0x2cu;
        c.edx -= 1u;
    } while (c.edx != 0u);

    c.edx = 0x2c3u;
    c.ebx = GP4_GLOBAL(uint32_t, 0x0081d1c8) + 0x12cu;
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d1cc) + 0x5cu;
    do {
        gp4::call_regs(0x0052800e, c);
        c.eax -= 0x2cu;
        c.edx -= 1u;
    } while (c.edx != 0x2c0u);

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, 0x2c0u));
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052a893, FUN_0052a893, "regs(edi:ptr[8]:bytes) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae+slot8_005388ba")
