#include <gp4/gp4.h>

// imul by a dword followed by shrd 26: low dword of (a * b) >> 26.
static inline uint32_t mulShr26(uint32_t a, uint32_t b) {
    const int64_t product = (int64_t)(int32_t)a * (int64_t)(int32_t)b;
    return (uint32_t)((uint64_t)product >> 26);
}

// esi = car state: dwords +0x2fc/+0x300/+0x304/+0x308 are four wheel-corner values; +0x38c/+0x390/+0x394/+0x398
// their offsets; byte +0x21d bit 5 a "clamp" request. The corner values are recomputed (either by 0x0041f1de
// from the four 0x300-byte records at 0x006380d8 when byte [0x629bd0] is 0xff, or from the [0x638034] /
// [0x638038] pitch and roll terms), and the change of each is stored in dword +0x174/+0x474/+0x774/+0xa74
// of the block at 0x006380d8 (corner order 2fc, 300, 304, 308), clamped to <= 0 after adding [0x6247e4]
// when the request bit is set. Ends in 0x0041f353, whose registers and flags it returns.
// FUNCTION: GP4 0x0041f489
void __cdecl FUN_0041f489(gp4::Regs* r) {
    uint8_t* state = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* block = GP4_ARRAY(uint8_t, 0x006380d8);

    const uint32_t old2fc = GP4_FIELD(uint32_t, state, 0x2fc);
    const uint32_t old300 = GP4_FIELD(uint32_t, state, 0x300);
    const uint32_t old304 = GP4_FIELD(uint32_t, state, 0x304);
    const uint32_t old308 = GP4_FIELD(uint32_t, state, 0x308);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = 0x006380d8u;

    if (GP4_GLOBAL(uint8_t, 0x00629bd0) == 0xffu) {
        gp4::call_regs(0x0041f1de, call);
    } else {
        const uint32_t base = ((GP4_GLOBAL(uint32_t, 0x00638044) - 0x6a00u) << 4) | (GP4_GLOBAL(uint32_t, 0x00638d58) >> 28);

        const uint32_t left = base - mulShr26(GP4_GLOBAL(uint32_t, 0x00638034), GP4_GLOBAL(uint32_t, 0x00638da4));
        const uint32_t leftSpread = mulShr26(GP4_GLOBAL(uint32_t, 0x00638038), GP4_GLOBAL(uint32_t, 0x00638dac));
        GP4_FIELD(uint32_t, state, 0x300) = left - leftSpread - GP4_FIELD(uint32_t, state, 0x390);
        GP4_FIELD(uint32_t, state, 0x2fc) = leftSpread + left - GP4_FIELD(uint32_t, state, 0x38c);

        const uint32_t right = mulShr26(GP4_GLOBAL(uint32_t, 0x00638034), GP4_GLOBAL(uint32_t, 0x00638da8)) + base;
        const uint32_t rightSpread = mulShr26(GP4_GLOBAL(uint32_t, 0x00638038), GP4_GLOBAL(uint32_t, 0x00638db0));
        GP4_FIELD(uint32_t, state, 0x308) = right - rightSpread - GP4_FIELD(uint32_t, state, 0x398);
        GP4_FIELD(uint32_t, state, 0x304) = rightSpread + right - GP4_FIELD(uint32_t, state, 0x394);
    }

    GP4_FIELD(uint32_t, block, 0xa74) = GP4_FIELD(uint32_t, state, 0x308) - old308;
    GP4_FIELD(uint32_t, block, 0x774) = GP4_FIELD(uint32_t, state, 0x304) - old304;
    GP4_FIELD(uint32_t, block, 0x474) = GP4_FIELD(uint32_t, state, 0x300) - old300;
    GP4_FIELD(uint32_t, block, 0x174) = GP4_FIELD(uint32_t, state, 0x2fc) - old2fc;

    if ((GP4_FIELD(uint8_t, state, 0x21d) & 0x20u) != 0u) {
        static const uint32_t offsets[4] = {0x174u, 0x474u, 0x774u, 0xa74u};
        for (int i = 0; i < 4; i++) {
            int32_t value = (int32_t)(GP4_FIELD(uint32_t, block, offsets[i]) + GP4_GLOBAL(uint32_t, 0x006247e4));
            if (value > 0) {
                value = 0;
            }
            GP4_FIELD(int32_t, block, offsets[i]) = value;
        }
        GP4_FIELD(uint8_t, state, 0x21d) = (uint8_t)(GP4_FIELD(uint8_t, state, 0x21d) & 0xdfu);
    }

    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0041f353, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0041f489, FUN_0041f489, "regs(esi:ptr[0x3a8]:i32[-16384..16384]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_006380d8+state_0041f489")
