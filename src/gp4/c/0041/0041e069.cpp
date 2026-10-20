#include <gp4/gp4.h>

// pushal wrapper that primes the per-lap record state. [0x00601f18] and [0x00602075] are cleared first. With
// the flag [0x00622424] set, the record 0x008943a0 is the source: its dword +0xf4 goes to [0x00623e34], the
// small state words 0x00602004/0x00601f1e/0x00602020/0x00602028 are cleared, [0x00602024] becomes 0x40000 and
// 0x0041ea15 runs on the record. Otherwise the three bytes 0x00627944/45/47 become 0xff, 0x00406dbb (eax =
// [0x00623e30], edx = [0x00623e38], edi = [0x00602008]) and 0x00407811 give the values stored to 0x00602018 /
// 0x006278c4 / 0x00623e34 (and to the slot [0x0062789c] * 0x300 + 0x006383a4 when that index is below 4),
// edi goes to 0x0060201c / 0x0062783c, then 0x0041dbb0 runs, [0x006278b4] / [0x006278bc] are copied to
// 0x00602020 / 0x00602024 and 0x0041dbdd runs. popal restores the registers; the flags are those of the last callee.
// FUNCTION: GP4 0x0041e069
void __cdecl FUN_0041e069(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.edi = GP4_GLOBAL(uint32_t, 0x00602008);
    GP4_GLOBAL(uint32_t, 0x00601f18) = 0;
    GP4_GLOBAL(uint8_t, 0x00602075) = 0;

    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0u) {
        c.edi = 0x008943a0u;
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
        c.eax = GP4_FIELD(uint32_t, record, 0xf4);
        GP4_GLOBAL(uint32_t, 0x00623e34) = c.eax;
        GP4_GLOBAL(uint32_t, 0x00602004) = 0;
        GP4_GLOBAL(uint8_t, 0x00601f1e) = 0;
        GP4_GLOBAL(uint32_t, 0x00601f18) = 0;
        GP4_GLOBAL(uint32_t, 0x00602020) = 0;
        c.eax = 0x40000u;
        GP4_GLOBAL(uint32_t, 0x00602024) = c.eax;
        GP4_GLOBAL(uint32_t, 0x00602028) = 0;
        gp4::call_regs(0x0041ea15, c);
    } else {
        GP4_GLOBAL(uint8_t, 0x00627945) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00627944) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00627947) = 0xffu;
        c.eax = GP4_GLOBAL(uint32_t, 0x00623e30);
        c.edx = GP4_GLOBAL(uint32_t, 0x00623e38);
        gp4::call_regs(0x00406dbb, c);
        GP4_GLOBAL(uint32_t, 0x00602018) = c.eax;
        GP4_GLOBAL(uint32_t, 0x0060201c) = c.edi;
        GP4_GLOBAL(uint32_t, 0x0062783c) = c.edi;
        gp4::call_regs(0x00407811, c);
        GP4_GLOBAL(uint32_t, 0x006278c4) = c.eax;
        GP4_GLOBAL(uint32_t, 0x00623e34) = c.eax;
        c.edx = GP4_GLOBAL(uint32_t, 0x0062789c);
        if (static_cast<int32_t>(c.edx) < 4) {
            c.edx = c.edx * 0x300u;
            *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.edx + 0x006383a4u)) = c.eax;
        }
        gp4::call_regs(0x0041dbb0, c);
        c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);
        GP4_GLOBAL(uint32_t, 0x00602020) = c.eax;
        c.eax = GP4_GLOBAL(uint32_t, 0x006278bc);
        GP4_GLOBAL(uint32_t, 0x00602024) = c.eax;
        gp4::call_regs(0x0041dbdd, c);
    }

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0041e069, FUN_0041e069, "regs() -> (cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=ring_008943a0+records_008943a0+state_004094e3+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_004381ab+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_0041dbb0+state_0040342a+tbl_00868598+steps_0041dada+state_0041d996+state_0041d9b6+mesh_00453330+state_0041d9eb+state_0041da34+ptr_00602008+ptr_00602008+win_0041e069")
