#include <gp4/gp4.h>
#include <gp4/flags.h>

// Allocates a block of eax bytes at the used end [0x82fcac] of the block pool (sibling of the top-down
// allocator 0x005389b9). The request is stored in [0x82fcb8]. When the serial [0x82fcb4] has reached
// 0x3fffff it restarts at 2 and every in-use slot's mark (+0x14) is set to 1 (0x00538991). The request
// fits while used end + size <= the limit [0x82fcb0] (unsigned) and a free slot exists (0x00538971):
// the slot then takes start = used end (+8), length = size (+0xc) and serial = [0x82fcb4] (+4), the used
// end grows by the size, the serial advances, and eax = slot dword +0 | serial << 10. Otherwise the slot
// 0x005388ed picks is evicted (0x00538921) and the test repeats; with nothing to evict eax = 0.
// ebx, ecx, esi and edi are saved and restored; the flags are those of `cmp eax, 0` (eax 0) or of the
// closing `or eax, ebx`.
// FUNCTION: GP4 0x005387dc
static inline uint32_t CallForEax(uint32_t addr, uint32_t eax_in) {
    gp4::Regs c = {};
    c.eax = eax_in;
    gp4::call_regs(addr, c);
    return c.eax;
}

void __cdecl FUN_005387dc(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082fcb8) = r->eax;
    if (GP4_GLOBAL(uint32_t, 0x0082fcb4) == 0x003fffffu) {
        GP4_GLOBAL(uint32_t, 0x0082fcb4) = 2u;
        gp4::Regs c = {};
        gp4::call_regs(0x00538991, c);
    }

    uint32_t result;
    uint32_t arith;
    for (;;) {
        const uint32_t wanted = GP4_GLOBAL(uint32_t, 0x0082fcb8) + GP4_GLOBAL(uint32_t, 0x0082fcac);
        if (wanted <= GP4_GLOBAL(uint32_t, 0x0082fcb0)) {
            const uint32_t slot = CallForEax(0x00538971, 0u);
            if (slot != 0u) {
                GP4_FIELD(uint32_t, slot, 8) = GP4_GLOBAL(uint32_t, 0x0082fcac);
                const uint32_t size = GP4_GLOBAL(uint32_t, 0x0082fcb8);
                GP4_FIELD(uint32_t, slot, 0xc) = size;
                GP4_GLOBAL(uint32_t, 0x0082fcac) = GP4_GLOBAL(uint32_t, 0x0082fcac) + size;
                GP4_FIELD(uint32_t, slot, 4) = GP4_GLOBAL(uint32_t, 0x0082fcb4);
                GP4_GLOBAL(uint32_t, 0x0082fcb4) = GP4_GLOBAL(uint32_t, 0x0082fcb4) + 1u;
                result = GP4_FIELD(uint32_t, slot, 0) | (GP4_FIELD(uint32_t, slot, 4) << 10);
                arith = gp4::flags::logic32(result);
                break;
            }
        }
        const uint32_t victim = CallForEax(0x005388ed, 0u);
        if (victim == 0u) {
            result = 0u;
            arith = gp4::flags::sub32(0u, 0u);
            break;
        }
        CallForEax(0x00538921, victim);
    }

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005387dc, FUN_005387dc, "regs(eax:u32{=0,1,2,3,4,5,8,0xc,0x10,0xfffffff0,0xffffffff}, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=pool_005387dc+serial_005387dc")
