#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the order-list bytes (0x00675958, [0x66d8ec] of them) with bit 7 set. The count is
// copied to 0x00934de4 first; the result (eax) is stored as a word in 0x007a93a8 and 0x007c2eb8.
// ebx/ecx are saved and restored. The flags are those of the last flag writer: "test ecx,ecx"
// for an empty list, otherwise the "inc ebx" closing the last iteration (CF kept from the
// preceding add/test, which is always 0).
// FUNCTION: GP4 0x00522a78
void __cdecl FUN_00522a78(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    GP4_GLOBAL(uint32_t, 0x00934de4) = count;
    const uint8_t* list = GP4_ARRAY(uint8_t, 0x00675958);

    uint32_t flagged = 0u;
    uint32_t arith = gp4::flags::logic32(count);
    for (uint32_t i = 0u; i < count; ++i) {
        if ((list[i] & 0x80u) != 0u) {
            flagged += 1u;
        }
        arith = gp4::flags::inc32(0x00675958u + i, 0u);
    }

    GP4_GLOBAL(uint16_t, 0x007a93a8) = (uint16_t)flagged;
    GP4_GLOBAL(uint16_t, 0x007c2eb8) = (uint16_t)flagged;

    r->eax = flagged;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522a78, FUN_00522a78, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=order_00675958+list_00443278")
