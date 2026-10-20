#include <gp4/gp4.h>

// FUNCTION: GP4 0x00424e41
// pushal ... popal routine. esi = object record: +0 / +8 are its x / z position and +0x64 its segment
// as a byte offset into the 0x160-stride ring at 0x008943a0. Clears the bytes 0x627944, 0x627945 and
// 0x627947, runs the segment locator 0x00406dbb (edi = ring record, eax = x, edx = z), writes the
// resulting segment offset back to +0x64, and stores the result of 0x00407811 (called with the
// locator's eax) to +0x58. All registers are restored; the flags left are those 0x00407811 leaves.
void __cdecl FUN_00424e41(gp4::Regs* r) {
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x00627945) = 0;
    GP4_GLOBAL(uint8_t, 0x00627944) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.edi = GP4_FIELD(uint32_t, object, 0x64) + 0x008943a0u;
    s.eax = GP4_FIELD(uint32_t, object, 0);
    s.edx = GP4_FIELD(uint32_t, object, 8);
    gp4::call_regs(0x00406dbb, s);

    s.edx = s.edi - 0x008943a0u;
    GP4_FIELD(uint32_t, object, 0x64) = s.edx;

    gp4::call_regs(0x00407811, s);
    GP4_FIELD(uint32_t, object, 0x58) = s.eax;

    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00424e41, FUN_00424e41, "regs(esi:ptr[0x6c]:layout=obj_00424e1b) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0")
