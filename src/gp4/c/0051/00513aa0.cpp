#include <gp4/gp4.h>
#include <gp4/flags.h>

// Branch [0x7ab1a0] == 0xff: only turn the offset tables back into pointers (0x5139ed).
static __declspec(noinline) void RelinkOnly(gp4::Regs& c) {
    gp4::call_regs(0x005139ed, c);
}

// Branch [0x7ab1a0] != 0xff: switch the groups off (0x441a4f), restore the state block from the
// buffer [0x7a93e0] (0x40b2a1, esi = buffer), relink (0x5139ed), copy [0x891504] to [0x679d20]
// (0x51ff93, eax = the value), rerun the ring/segment setup (0x441a5c) and reset the four words
// 0x66d882..0x66d888 (0x51ffe2).
static __declspec(noinline) void RestoreAndRelink(gp4::Regs& c) {
    gp4::call_regs(0x00441a4f, c);
    c.esi = GP4_GLOBAL(uint32_t, 0x007a93e0);
    gp4::call_regs(0x0040b2a1, c);
    gp4::call_regs(0x005139ed, c);
    gp4::call_regs(0x0051ff93, c);
    gp4::call_regs(0x00441a5c, c);
    gp4::cdecl_<0x0051ffe2, void>();
}

// Plain register routine (no pushal): rebases the pointer tables to offsets (0x513982), resets the
// row/fill tables (0x5204c0), clears a block of session globals, sets the five bytes 0x679d18.. to
// 0xff (0x51d4e5), copies the state block into the buffer [0x7a93e0] (0x40b264, eax = the copied
// size, moved to ecx), runs dispatcher command 0x1f (0x531f9c), then either just relinks the tables
// ([0x7ab1a0] == 0xff) or restores the state from the buffer and relinks, and finally runs 0x553941
// and clears [0x7a93b7]. edi is left = [0x7a93e0] (esi too on the restore path), ecx = the copied size,
// eax and the flags are those the callees leave.
// FUNCTION: GP4 0x00513aa0
void __cdecl FUN_00513aa0(gp4::Regs* r) {
    gp4::Regs c = *r;

    gp4::call_regs(0x00513982, c);
    gp4::call_regs(0x005204c0, c);

    GP4_GLOBAL(uint16_t, 0x007c2e94) = 0x8000;
    GP4_GLOBAL(uint32_t, 0x007c2f28) = 0;
    GP4_GLOBAL(uint8_t, 0x008914e3) = 0;
    GP4_GLOBAL(uint8_t, 0x007a90fc) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d7a8) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d818) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d84c) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d89f) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d8a0) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d8a1) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d8a2) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d8a4) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d8b0) = 0;
    gp4::cdecl_<0x0051d4e5, void>();

    c.edi = GP4_GLOBAL(uint32_t, 0x007a93e0);
    gp4::call_regs(0x0040b264, c);
    c.ecx = c.eax;
    c.eax = (c.eax & 0xffff0000u) | 0x1fu;
    gp4::call_regs(0x00531f9c, c);

    if (GP4_GLOBAL(uint8_t, 0x007ab1a0) == 0xffu) {
        RelinkOnly(c);
    } else {
        RestoreAndRelink(c);
    }

    gp4::call_regs(0x00553941, c);
    GP4_GLOBAL(uint8_t, 0x007a93b7) = 0;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00513aa0, FUN_00513aa0, "regs(ebx:u32, edx:u32, esi:u32, ebp:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=links_0066fef0+ptrs_0066d818+rows_00520584+fill_005204d0+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70+list_00549db9+script_all_00835846+sbuf_00513aa0; maxinsn=8000000; wall=3600")
