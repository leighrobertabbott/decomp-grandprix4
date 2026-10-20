#include <gp4/gp4.h>
#include <gp4/flags.h>

// Allocates the first free record of the 0x140 x 0xe0-byte array at 0x00854414 (0x00552762, eax = index,
// edx = index * 0xe0, CF set when all are in use) and initialises it: +0x98 = 0x2180, +0x74 = esi, +0xdc =
// ecx (also kept in [0x00866e88]); 0x00424371 and 0x004243cd copy the object-slot data of that slot
// (eax = record), then +0x50 = 0x2ee00000, +0x54 = 1, +0x55 = 0, +0x56 = 0, +0x90 = -1, +0x48 = the
// pointer 0x00424e02 gives for the slot ecx, and bit 0 of +0x4c is set (active). The whole body is a
// pushal/popal block, so the registers are unchanged; the exit is stc when no record was free (flags of
// 0x00552762 with CF set) and otherwise clc after the final `or dword [edi+0x4c], 1`.
// FUNCTION: GP4 0x00552bea
void __cdecl FUN_00552bea(gp4::Regs* r) {
    const uint32_t slot = r->ecx;
    GP4_GLOBAL(uint32_t, 0x00866e88) = slot;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00552762, c);
    if ((c.eflags & gp4::CF) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags | gp4::CF);
        return;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edx + 0x00854414u));
    GP4_FIELD(uint32_t, record, 0x98) = 0x2180u;
    GP4_FIELD(uint32_t, record, 0x74) = r->esi;
    GP4_FIELD(uint32_t, record, 0xdc) = GP4_GLOBAL(uint32_t, 0x00866e88);

    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    c.eax = c.edi;
    gp4::call_regs(0x00424371, c);
    gp4::call_regs(0x004243cd, c);

    GP4_FIELD(uint32_t, record, 0x50) = 0x2ee00000u;
    GP4_FIELD(uint8_t, record, 0x54) = 1u;
    GP4_FIELD(uint8_t, record, 0x55) = 0u;
    GP4_FIELD(uint16_t, record, 0x56) = 0u;
    GP4_FIELD(uint32_t, record, 0x90) = 0xffffffffu;

    c.eax = GP4_GLOBAL(uint32_t, 0x00866e88);
    gp4::call_regs(0x00424e02, c);
    GP4_FIELD(uint32_t, record, 0x48) = c.eax;
    const uint32_t active = GP4_FIELD(uint32_t, record, 0x4c) | 1u;
    GP4_FIELD(uint32_t, record, 0x4c) = active;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(active));
}
GP4_IMPL(0x00552bea, FUN_00552bea, "regs(ecx:u32[0..15], esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tbl_00854460+recs_0063bd7c")
