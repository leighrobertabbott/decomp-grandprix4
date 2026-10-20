#include <gp4/gp4.h>

// edi: a body record; its dword +0x44 is the reference segment address.
struct Unknown_0040d106_Body {
    uint8_t Unknown00[0x44];
    uint32_t Unknown44;
};

// FUNCTION: GP4 0x0040d106
// Scans the segment records esi, esi + 0x160, ... up to (excluding) ebx. For each record it clears
// [0x62793c] / [0x627940] and runs the corner test 0x0040d01b (which sets CF when the record has to be
// counted); counted records give the signed segment offset (esi - [edi + 0x44]) / 0x160 (0x0040cfba), which
// updates the maximum [0x627904] and the minimum [0x627900]. Bytes 0x627945 / 0x627947 are cleared once.
// pushal / popal: every register is preserved; the loop ends with esi == ebx, so the flags are those of
// the final equal compare (ZF = PF = 1).
void __cdecl FUN_0040d106(gp4::Regs* r) {
    const Unknown_0040d106_Body* body = reinterpret_cast<const Unknown_0040d106_Body*>(static_cast<uintptr_t>(r->edi));
    uint32_t segment = r->esi;
    const uint32_t end = r->ebx;

    GP4_GLOBAL(uint8_t, 0x00627945) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0;
    do {
        GP4_GLOBAL(uint32_t, 0x0062793c) = 0;
        GP4_GLOBAL(uint32_t, 0x00627940) = 0;

        gp4::Regs test = *r;
        test.esi = segment;
        test.st_in = 0;
        test.st_out = 0;
        gp4::call_regs(0x0040d01b, test);

        if ((test.eflags & 1u) != 0u) {
            gp4::Regs divide = *r;
            divide.eax = body->Unknown44;
            divide.edx = segment;
            divide.st_in = 0;
            divide.st_out = 0;
            gp4::call_regs(0x0040cfba, divide);
            const int32_t offset = static_cast<int32_t>(divide.eax);
            if (GP4_GLOBAL(int32_t, 0x00627904) < offset) {
                GP4_GLOBAL(int32_t, 0x00627904) = offset;
            }
            if (GP4_GLOBAL(int32_t, 0x00627900) > offset) {
                GP4_GLOBAL(int32_t, 0x00627900) = offset;
            }
        }
        segment += 0x160u;
    } while (segment != end);

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040d106, FUN_0040d106, "regs(edi:rec[0x8943a0,0x160,0..27], esi:rec[0x8943a0,0x160,0..13], ebx:rel[esi,0x160,1..11]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+ptr_00627938+scan_00406d4b+lap_0040cf8c")
