#include <gp4/gp4.h>

// FUNCTION: GP4 0x004250b9
void __cdecl FUN_004250b9(gp4::Regs* r) {
    const uint8_t* block = (const uint8_t*)r->esi;
    uint32_t packed = *(const uint32_t*)(block + 0x74);

    // cmp ebp,-1 (equal) and the loop's final cmp ecx,4 (equal) both leave ZF/PF set
    const uint32_t flags = 0x44u;

    if (packed == 0xffffffffu) {
        r->ebp = packed;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    uint32_t byteValue = r->eax;
    uint32_t slot = r->edx;
    for (int i = 0; i < 4; ++i) {
        byteValue = packed & 0xffu;
        if (byteValue != 0xffu) {
            slot = GP4_GLOBAL(uint32_t, 0x00609864);
            GP4_ARRAY(uint32_t, 0x00609870)[slot] = byteValue;
            GP4_GLOBAL(uint32_t, 0x00609864) = GP4_GLOBAL(uint32_t, 0x00609864) + 1u;
        }
        packed >>= 8;
    }

    r->eax = byteValue;
    r->ecx = 4u;
    r->edx = slot;
    r->ebp = packed;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004250b9, FUN_004250b9, "regs(esi:ptr[0x7c]:layout=rec_004250b9) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004250b9")
