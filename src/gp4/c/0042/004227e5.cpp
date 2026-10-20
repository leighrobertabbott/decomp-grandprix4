#include <gp4/gp4.h>

// Fills the 0x20-byte header of the entry block behind DAT_006097b0: the point
// (DAT_0062a064/68/6c) scaled by 32, the three dwords DAT_0062a070/74/78 unchanged, a zero at
// +0x18 (the entry count) and, at +0x1c, the result of 0x00422501 (largest distance to the
// entries). Leaves edi = the block pointer; eax and the flags are the callee's (cmp result, 0x80).
// FUNCTION: GP4 0x004227e5
void __cdecl FUN_004227e5(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006097b0)));

    GP4_FIELD(uint32_t, block, 0x00) = GP4_GLOBAL(uint32_t, 0x0062a064) << 5;
    GP4_FIELD(uint32_t, block, 0x04) = GP4_GLOBAL(uint32_t, 0x0062a068) << 5;
    GP4_FIELD(uint32_t, block, 0x08) = GP4_GLOBAL(uint32_t, 0x0062a06c) << 5;
    GP4_FIELD(uint32_t, block, 0x0c) = GP4_GLOBAL(uint32_t, 0x0062a070);
    GP4_FIELD(uint32_t, block, 0x10) = GP4_GLOBAL(uint32_t, 0x0062a074);
    GP4_FIELD(uint32_t, block, 0x14) = GP4_GLOBAL(uint32_t, 0x0062a078);
    GP4_FIELD(uint32_t, block, 0x18) = 0u;

    gp4::Regs call = *r;
    call.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block));
    gp4::call_regs(0x00422501, call);

    GP4_FIELD(uint32_t, block, 0x1c) = call.eax;

    *r = call;
}
GP4_IMPL(0x004227e5, FUN_004227e5, "regs() -> (eax, edi, cf, pf, af, zf, sf, of); globals=layout=state_00422397+list_0062a048")
