#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct PoolSlot_00829cac {
    uint8_t Unknown00[0xc];
    uint32_t InUse0c;
    uint32_t Unknown10;
    uint32_t Mark14;
};
#pragma pack(pop)

// Walks the 0x400 slots (0x18 bytes each) of the block pool at 0x00829cac and sets the dword +0x14
// to 1 in every slot whose dword +0xc (the block length, nonzero = in use) is nonzero. ecx and edi
// are saved and restored; the flags are those of the final `cmp ecx, 0x400` (equal).
// FUNCTION: GP4 0x00538991
void __cdecl FUN_00538991(gp4::Regs* r) {
    PoolSlot_00829cac* slot = GP4_ARRAY(PoolSlot_00829cac, 0x00829cac);
    for (uint32_t i = 0; i < 0x400u; ++i, ++slot) {
        if (slot->InUse0c != 0u) {
            slot->Mark14 = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x400u, 0x400u));
}
GP4_IMPL(0x00538991, FUN_00538991, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=full_00538971")
