#include <gp4/gp4.h>

// Maps a driver id (al) to a Q14 value. An id whose slot ((id & 0x3f) - 1) in the byte table at
// 0x7c2a74 holds 2 returns 0x4000 at once. Otherwise the id is looked up by 0x409b17 (index >> 1,
// stored in [0x6077d0]); an id with bit 7 set gets 0x4000 when one of the special selectors matches
// ([0x7ab020] == 2, id == [0x66d896], or [0x7aafd4] != 0 and id == [0x66d87d]) and the result of
// 0x43cdf8 for the word [0x7ad89e] otherwise; a plain id reads the word table at 0x66d7d6 (or
// 0x66d7ec when [0x7c2ea9] has bit 6) at the looked-up index. The value and the id (dl) then go
// through 0x43ce04. ebx, ecx, edx, ebp, esi and edi are saved and restored; the flags are the
// callee's (or the equal compare of the early exit).
// FUNCTION: GP4 0x0043ce1d
void __cdecl FUN_0043ce1d(gp4::Regs* r) {
    const uint32_t input = r->eax;
    const uint8_t id = static_cast<uint8_t>(input);
    GP4_GLOBAL(uint8_t, 0x006077ce) = id;

    const int32_t slot = static_cast<int32_t>(input & 0x3fu) - 1;
    if (GP4_ARRAY(uint8_t, 0x007c2a74)[slot] == 2u) {
        r->eax = 0x4000u;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;   // CMP of equal bytes: ZF = PF = 1
        return;
    }

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ecx = static_cast<uint32_t>(slot);
    gp4::call_regs(0x00409b17, s);
    const uint32_t index = s.eax;
    GP4_GLOBAL(uint32_t, 0x006077d0) = index;

    uint32_t value;
    if (id & 0x80u) {
        const uint8_t special = GP4_GLOBAL(uint8_t, 0x007ab020);
        if (special == 2u
            || id == GP4_GLOBAL(uint8_t, 0x0066d896)
            || (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && id == GP4_GLOBAL(uint8_t, 0x0066d87d))) {
            value = 0x4000u;
        } else {
            s.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
            gp4::call_regs(0x0043cdf8, s);
            value = s.eax;
        }
    } else {
        s.ebx = GP4_GLOBAL(uint32_t, 0x006077d0);
        if (GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u) {
            value = GP4_ARRAY(uint16_t, 0x0066d7ec)[s.ebx];
        } else {
            value = GP4_ARRAY(uint16_t, 0x0066d7d6)[s.ebx];
        }
    }

    s.eax = value;
    s.edx = (s.edx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x006077ce);
    gp4::call_regs(0x0043ce04, s);
    r->eax = s.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0043ce1d, FUN_0043ce1d, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c+state_0042595c+state_0066d8c0")
