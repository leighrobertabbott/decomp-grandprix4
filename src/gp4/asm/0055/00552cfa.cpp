#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the first and third dword of the source record at edx into the object record eax (a 0xe0 byte
// record of the array at 0x00866248: +0 and +8), then runs the object update 0x0055278a with ebx = that
// record (eax and edx are restored, ebx is saved and restored). Flags and registers are what the update
// leaves.
// FUNCTION: GP4 0x00552cfa
void __cdecl FUN_00552cfa(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t source = r->edx;
    const uint32_t record = index * 0xe0u + 0x00866248u;
    GP4_FIELD(uint32_t, record, 0) = GP4_FIELD(uint32_t, source, 0);
    GP4_FIELD(uint32_t, record, 8) = GP4_FIELD(uint32_t, source, 8);

    gp4::Regs c = *r;
    c.ebx = record;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(record - 0x00866248u, 0x00866248u));
    gp4::call_regs(0x0055278a, c);
    c.ebx = r->ebx;
    *r = c;
}
GP4_IMPL(0x00552cfa, FUN_00552cfa, "regs(eax:u32{=0,1,3,6,10}, ebx:u32, edx:ptr[0x10]:i32[-0x400..0x400], esi:ptr[0xe0]:layout=fxowner_0055278a) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=tbl_00854460+trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0+fx_00866248")
