#include <gp4/gp4.h>

// Reads two big-endian words from the bytes at esi (lodsb x4, esi += 4): the first goes to ebx, the
// second to eax (both movzx), then calls the hand-written routine 0x0054cb7e with them. That
// routine keeps every register; the flags are the ones it leaves behind.
// FUNCTION: GP4 0x0054d31a
void __cdecl FUN_0054d31a(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t first = ((uint32_t)src[0] << 8) | src[1];
    const uint32_t second = ((uint32_t)src[2] << 8) | src[3];

    gp4::Regs c = *r;
    c.esi = r->esi + 4u;
    c.ebx = first;
    c.eax = second;
    gp4::call_regs(0x0054cb7e, c);

    r->esi = c.esi;
    r->ebx = c.ebx;
    r->eax = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054d31a, FUN_0054d31a, "regs(esi:ptr[0x40]:u8{0,1,2,3,0x58,0x80,0xff}) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=layout=lim_0054d31a")
