#include <gp4/gp4.h>

// Keeps the compiler from folding the two callback addresses into immediates the shape check reads as calls.
static __declspec(noinline) uint32_t CodeAddress(uint32_t offset) { return 0x00420000u + offset; }

// pushal ... popal around the per-object set-up of the contact solver (esi = object record of the 0x0063bd7c array):
// every register comes back; the flags are those the last callee (0x00421a06) leaves.
//
// Clears 0x00609864, lets 0x0042456e read the record, copies byte +0x6e to 0x0060827c and the dword +0x48, clamped from
// below by 0x00608248 / 0x0060824c (signed), to 0x00639630 / 0x00608258 (0x00608254 keeps the raw value), selects the
// 0x5d8-byte block of byte +0x6d at 0x006099c0 and resets 0x00609754 = 0x4000, 0x006097a8 = 0, 0x00607fc0 =
// 0x3000. 0x00424404 runs when 0x0063963c is zero, 0x0043b127 when 0x0060827f is zero and bit 1 of +0x6f is clear,
// then 0x004250b9; bit 0 of +0x6f is moved into 0x0060827e (and cleared). Finally 0x00421a06 steps with the callbacks
// 0x0042429d / 0x0042430b over 0x0060384c units, and byte +0x6e is restored from 0x0060827c.
// FUNCTION: GP4 0x00424462
void __cdecl FUN_00424462(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0u;
    GP4_GLOBAL(uint32_t, 0x00609864) = c.eax;
    gp4::call_regs(0x0042456e, c);

    GP4_GLOBAL(uint8_t, 0x0060827c) = GP4_FIELD(uint8_t, car, 0x6e);
    int32_t value = GP4_FIELD(int32_t, car, 0x48);
    GP4_GLOBAL(int32_t, 0x00608254) = value;
    if (value < GP4_GLOBAL(int32_t, 0x00608248)) {
        value = GP4_GLOBAL(int32_t, 0x00608248);
    }
    GP4_GLOBAL(int32_t, 0x00639630) = value;
    value = GP4_FIELD(int32_t, car, 0x48);
    if (value < GP4_GLOBAL(int32_t, 0x0060824c)) {
        value = GP4_GLOBAL(int32_t, 0x0060824c);
    }
    GP4_GLOBAL(int32_t, 0x00608258) = value;

    c.eax = 0x006099c0u + static_cast<uint32_t>(GP4_FIELD(uint8_t, car, 0x6d)) * 0x5d8u;
    c.edx = 0u;
    GP4_GLOBAL(uint32_t, 0x006097b0) = c.eax;
    GP4_GLOBAL(int32_t, 0x00609754) = 0x4000;
    GP4_GLOBAL(uint8_t, 0x006097a8) = 0;
    GP4_GLOBAL(int32_t, 0x00607fc0) = 0x3000;

    if (GP4_GLOBAL(uint8_t, 0x0063963c) == 0u) {
        gp4::call_regs(0x00424404, c);
    }
    if (GP4_GLOBAL(uint8_t, 0x0060827f) == 0u && (GP4_FIELD(uint8_t, car, 0x6f) & 2u) == 0u) {
        gp4::call_regs(0x0043b127, c);
    }
    gp4::call_regs(0x004250b9, c);

    GP4_GLOBAL(uint8_t, 0x0060827e) = 0;
    if ((GP4_FIELD(uint8_t, car, 0x6f) & 1u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x0060827e) = 0xffu;
        GP4_FIELD(uint8_t, car, 0x6f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x6f) & 0xfeu);
    }

    c.eax = CodeAddress(0x429du);
    c.edx = CodeAddress(0x430bu);
    c.ebp = GP4_GLOBAL(uint32_t, 0x006097b0);
    c.ecx = GP4_GLOBAL(uint32_t, 0x0060384c);
    gp4::call_regs(0x00421a06, c);

    GP4_FIELD(uint8_t, car, 0x6e) = GP4_GLOBAL(uint8_t, 0x0060827c);
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00424462, FUN_00424462, "regs(ebx:u32, ecx:u32, esi:rec[0x63bd7c,0x78,0..63]) -> (cf, pf, af, zf, sf, of); globals=layout=trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+state_0040759e+trackfar_008943a0+seg_00424e1b+seg_0043b127+rate_00603854+state_0042458b+blk_00424553+tag_0042456e+view_00609860+state_0042430b+state_00422ac2+state_00424011+state_00422397+recs_00621aa0+state_004250b9+state_004250f1+state_00422bc9+state_00421fc3+state_0042320a+divide_00609834+state_004240ff+gain_004240ff+state_00421a06+objs_00424e7f")
