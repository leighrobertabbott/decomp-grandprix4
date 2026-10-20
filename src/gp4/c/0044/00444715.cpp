#include <gp4/gp4.h>

// Rescales the mass-property results of 0x00444517 (centroid 0x0062a064..6c, inertias 0x0062a070..78, mass
// 0x0062a07c) by the run-time car constants: centroid += offsets 0x0060135c..64 - reference 0x0062a084..8c,
// every other value = value * scale (0x00601370/74/78/88) / reference (0x0062a090/94/98/9c).
// pushal / popal restore every register. popal does not restore the flags: they stay those of the last
// flag writer, the 'imul dword ptr [0x601388]' of the mass term (idiv leaves the flags alone).
// FUNCTION: GP4 0x00444715
void __cdecl FUN_00444715(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x0062a064) =
        (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x0060135c) + (uint32_t)GP4_GLOBAL(int32_t, 0x0062a064) -
                  (uint32_t)GP4_GLOBAL(int32_t, 0x0062a084));
    GP4_GLOBAL(int32_t, 0x0062a068) =
        (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00601360) + (uint32_t)GP4_GLOBAL(int32_t, 0x0062a068) -
                  (uint32_t)GP4_GLOBAL(int32_t, 0x0062a088));
    GP4_GLOBAL(int32_t, 0x0062a06c) =
        (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00601364) + (uint32_t)GP4_GLOBAL(int32_t, 0x0062a06c) -
                  (uint32_t)GP4_GLOBAL(int32_t, 0x0062a08c));

    int32_t rem;
    GP4_GLOBAL(int32_t, 0x0062a070) = gp4::idiv64(
        (int64_t)GP4_GLOBAL(int32_t, 0x0062a070) * (int64_t)GP4_GLOBAL(int32_t, 0x00601370),
        GP4_GLOBAL(int32_t, 0x0062a090), &rem);
    GP4_GLOBAL(int32_t, 0x0062a074) = gp4::idiv64(
        (int64_t)GP4_GLOBAL(int32_t, 0x0062a074) * (int64_t)GP4_GLOBAL(int32_t, 0x00601374),
        GP4_GLOBAL(int32_t, 0x0062a094), &rem);
    GP4_GLOBAL(int32_t, 0x0062a078) = gp4::idiv64(
        (int64_t)GP4_GLOBAL(int32_t, 0x0062a078) * (int64_t)GP4_GLOBAL(int32_t, 0x00601378),
        GP4_GLOBAL(int32_t, 0x0062a098), &rem);

    const int64_t massProduct = (int64_t)GP4_GLOBAL(int32_t, 0x0062a07c) * (int64_t)GP4_GLOBAL(int32_t, 0x00601388);
    GP4_GLOBAL(int32_t, 0x0062a07c) = gp4::idiv64(massProduct, GP4_GLOBAL(int32_t, 0x0062a09c), &rem);

    // Flags of the final imul: CF = OF = the 64-bit product does not fit 32 bits; PF/ZF/SF from the low half.
    const uint32_t low = (uint32_t)massProduct;
    uint32_t parity = low & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    uint32_t flags = 0;
    if (massProduct != (int64_t)(int32_t)low) flags |= 0x001u | 0x800u;  // CF | OF
    if (!(parity & 1u)) flags |= 0x004u;                                   // PF
    if (low == 0) flags |= 0x040u;                                         // ZF
    if (low & 0x80000000u) flags |= 0x080u;                                // SF
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444715, FUN_00444715, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_00444517")
