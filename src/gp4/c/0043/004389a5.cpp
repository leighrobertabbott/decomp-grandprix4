#include <gp4/gp4.h>

// Tries each of the 23 entries of the condition tables at 0x0062a0a4 (mask), 0x0062a1b8 and
// 0x0062a214 (lower bounds on the scaled length) and 0x0062a100 (bits to set) against the mask in
// eax and the length of the vector (DAT_00629dd4/d8/dc >> 8) packed by 0x00438901; every matching
// entry adds its bits to the dword at +0x40 of the record in esi and stores the packed vector at
// +0x48. When the dword is still zero, 0x004378fc must first accept the record (CF clear).
// pushal ... popal: every GPR survives; the flags are those of the final `cmp ebx, 0x17`, or of the
// refusing call to 0x004378fc.
// FUNCTION: GP4 0x004389a5
void __cdecl FUN_004389a5(gp4::Regs* r) {
    const uint32_t mask = r->eax;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs vector = *r;
    vector.st_in = 0;
    vector.st_out = 0;
    vector.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00629dd4) >> 8);
    vector.ebx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00629dd8) >> 8);
    vector.ecx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00629ddc) >> 8);
    gp4::call_regs(0x00438901, vector);
    const uint32_t packed = vector.eax;
    GP4_GLOBAL(uint32_t, 0x00629de0) = packed;

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(packed >> 24))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629f40));
    const int32_t scaled = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);

    const uint32_t* masks = GP4_ARRAY(uint32_t, 0x0062a0a4);
    const int32_t* lowerA = GP4_ARRAY(int32_t, 0x0062a214);
    const int32_t* lowerB = GP4_ARRAY(int32_t, 0x0062a1b8);
    const uint32_t* bits = GP4_ARRAY(uint32_t, 0x0062a100);

    uint32_t flags = 0x44u;   // cmp ebx, 0x17 with ebx == 0x17
    for (uint32_t i = 0; i != 0x17u; i++) {
        if ((masks[i] & mask) == 0u) continue;
        if (scaled < lowerA[i]) continue;
        if (GP4_GLOBAL(uint8_t, 0x00629f30) != 0xffu && scaled < lowerB[i]) continue;

        const uint32_t add = ~GP4_FIELD(uint32_t, record, 0x40) & bits[i];
        if (GP4_FIELD(uint32_t, record, 0x40) == 0u) {
            if (add == 0u) continue;
            gp4::Regs check = *r;
            check.st_in = 0;
            check.st_out = 0;
            check.eax = add;
            check.edx = packed;
            gp4::call_regs(0x004378fc, check);
            if ((check.eflags & 1u) != 0u) {
                flags = check.eflags & 0x8d5u;
                break;
            }
        }
        GP4_FIELD(uint32_t, record, 0x40) |= add;
        GP4_FIELD(uint32_t, record, 0x48) = packed;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004389a5, FUN_004389a5, "regs(eax:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
