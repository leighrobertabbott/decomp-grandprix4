#include <gp4/gp4.h>
#include <gp4/flags.h>

// Slot address of the saved row: [0x007a8e18] + [0x007a9144] * [0x006975c4] (`mul edx; add esi, eax`,
// the low half of the product).
static __declspec(noinline) uint32_t RowAddress() {
    return GP4_GLOBAL(uint32_t, 0x007a8e18) + GP4_GLOBAL(uint32_t, 0x007a9144) * GP4_GLOBAL(uint32_t, 0x006975c4);
}

// Appends the saved state to the record buffer at edi. Byte [0x007c2e72] == 0xff: copies one
// [0x006975c4]-byte row of the slot area (row [0x007a9144]) to edi and then appends the stream span
// with 0x00512eef. Otherwise, unless the byte [0x007c2ea5] is set, appends the walked slot rows with
// 0x00512d68 and then the circular byte buffer with 0x00512e82 (esi starts at the slot area). edx is
// saved and restored; the other registers and the flags are those the last callee leaves (when
// [0x007c2ea5] is set only esi changes and the flags are those of `cmp byte [0x007c2ea5], 0`).
// FUNCTION: GP4 0x00512eac
void __cdecl FUN_00512eac(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x007a8e18);

    if (GP4_GLOBAL(uint8_t, 0x007c2e72) == 0xffu) {
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x006975c4);
        uint32_t source = RowAddress();
        uint32_t target = c.edi;
        for (uint32_t n = count; n != 0u; --n) {
            GP4_FIELD(uint8_t, target, 0) = GP4_FIELD(uint8_t, source, 0);
            ++source;
            ++target;
        }
        c.esi = source;
        c.edi = target;
        c.ecx = 0u;
        gp4::call_regs(0x00512eef, c);
    } else {
        const uint8_t blocked = GP4_GLOBAL(uint8_t, 0x007c2ea5);
        if (blocked != 0u) {
            r->esi = c.esi;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(blocked, 0u));
            return;
        }
        gp4::call_regs(0x00512d68, c);
        gp4::call_regs(0x00512e82, c);
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00512eac, FUN_00512eac, "regs(edi:ptr[0x50000]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00512cc1+idx_00520b65+ring_00512d68+ring_00512e82+cache_0053832b+save_00512eac")
