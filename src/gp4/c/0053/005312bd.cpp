#include <gp4/gp4.h>
#include <gp4/flags.h>

// Settings-column refresh for the three 0x3c-byte device records at 0x00827160. Each record is walked with
// ebx = the record and ecx = its index; 0x00531340 counts the set bits of the column mask byte +6 (eax). A
// record whose state byte +5 is 1, or is 4 with fewer than two mask bits set, gets its empty setting columns
// refilled by 0x005312f6 (a pushal wrapper) and its state byte is then set to 4. On return ebx is one record
// past the table (0x00827214), ecx = 3, eax is the last record's mask bit count and the flags are those of the
// final `cmp ecx, 3`.
// FUNCTION: GP4 0x005312bd
void __cdecl FUN_005312bd(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebx = 0x00827160u;
    c.ecx = 0u;

    do {
        gp4::call_regs(0x00531340, c);
        const uint8_t state = GP4_FIELD(uint8_t, c.ebx, 5);
        if (state == 1u || (state == 4u && c.eax <= 1u)) {
            gp4::call_regs(0x005312f6, c);
            GP4_FIELD(uint8_t, c.ebx, 5) = 4u;
        }
        c.ebx += 0x3cu;
        c.ecx += 1u;
    } while (c.ecx != 3u);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(c.ecx, 3u));
}
GP4_IMPL(0x005312bd, FUN_005312bd, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_00530f3f+dev_0053106d+mask6_00531381")
