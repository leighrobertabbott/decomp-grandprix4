#include <gp4/gp4.h>

// pushal ... popal: every register comes back; the flags of the last writer stay, here the final
// "cmp ebx, [0x627b98]" of the second pass with equal operands (ZF and PF set).
//
// Smooths the byte grid (DAT_00627b94 bytes, rows of DAT_00627b98): fills it at 0x00627bbc with
// the bytes of the random generator 0x00401462 (seeded from the words at 0x00629bca/cc/ce, the
// generator state at 0x0066d8be/c0/c2 is restored afterwards), copies it to 0x00628bc0, runs the
// horizontal filter 0x0041de96 over every (column, row) and, after another copy, the vertical
// filter 0x0041dee8 over every (row, column).
// FUNCTION: GP4 0x0041df3f
void __cdecl FUN_0041df3f(gp4::Regs* r) {
    const uint16_t savedBe = GP4_GLOBAL(uint16_t, 0x0066d8be);
    const uint16_t savedC0 = GP4_GLOBAL(uint16_t, 0x0066d8c0);
    const uint16_t savedC2 = GP4_GLOBAL(uint16_t, 0x0066d8c2);
    GP4_GLOBAL(uint16_t, 0x0066d8be) = GP4_GLOBAL(uint16_t, 0x00629bca);
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = GP4_GLOBAL(uint16_t, 0x00629bcc);
    GP4_GLOBAL(uint16_t, 0x0066d8c2) = GP4_GLOBAL(uint16_t, 0x00629bce);

    uint8_t* source = GP4_ARRAY(uint8_t, 0x00627bbc);
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = (r->eax & 0xffff0000u) | savedC2;
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x00627b94);
    do {
        gp4::call_regs(0x00401462, call);
        *source = static_cast<uint8_t>(call.eax);
        ++source;
        --remaining;
    } while (remaining != 0u);

    GP4_GLOBAL(uint16_t, 0x0066d8c2) = savedC2;
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = savedC0;
    GP4_GLOBAL(uint16_t, 0x0066d8be) = savedBe;

    uint8_t* grid = GP4_ARRAY(uint8_t, 0x00628bc0);
    for (uint32_t i = 0; i < GP4_GLOBAL(uint32_t, 0x00627b94); ++i) {
        grid[i] = GP4_ARRAY(uint8_t, 0x00627bbc)[i];
    }

    uint32_t row = 0;
    do {
        uint32_t column = 0;
        do {
            call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.ebx = column;
            call.edi = row;
            gp4::call_regs(0x0041de96, call);
            ++column;
        } while (column != GP4_GLOBAL(uint32_t, 0x00627b98));
        row += GP4_GLOBAL(uint32_t, 0x00627b98);
    } while (row != GP4_GLOBAL(uint32_t, 0x00627b94));

    for (uint32_t i = 0; i < GP4_GLOBAL(uint32_t, 0x00627b94); ++i) {
        grid[i] = GP4_ARRAY(uint8_t, 0x00627bbc)[i];
    }

    uint32_t column = 0;
    do {
        row = 0;
        do {
            call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.ebx = column;
            call.edi = row;
            gp4::call_regs(0x0041dee8, call);
            row += GP4_GLOBAL(uint32_t, 0x00627b98);
        } while (row != GP4_GLOBAL(uint32_t, 0x00627b94));
        ++column;
    } while (column != GP4_GLOBAL(uint32_t, 0x00627b98));

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041df3f, FUN_0041df3f, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=grid_00628bc0+loops_0041df3f; seed=1")
