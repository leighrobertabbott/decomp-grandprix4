#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inserts the byte in al into the 22-byte list at 0x00675958: the destination slot is
// clamp(dl - 1, 0, 0x15); the bytes from that slot upwards are shifted one place towards the end
// (the last one is dropped). Register results: ebx = edx = the slot, ah = the byte last copied
// (unchanged when no byte was shifted), flags = those of the final `cmp ebx, edx` (equal).
// FUNCTION: GP4 0x00522cda
void __cdecl FUN_00522cda(gp4::Regs* r) {
    const uint32_t eax_in = r->eax;
    const uint8_t value = (uint8_t)eax_in;
    uint8_t carried = (uint8_t)(eax_in >> 8);

    int16_t slot = (int16_t)(uint16_t)((r->edx & 0xffu) - 1u);
    if (slot < 0) {
        slot = 0;
    }
    if (slot > 0x15) {
        slot = 0x15;
    }
    const uint32_t limit = (uint32_t)(int32_t)slot;

    uint8_t* list = GP4_ARRAY(uint8_t, 0x00675958);
    uint32_t i = 0x15;
    while (i > limit) {
        carried = list[i - 1];
        list[i] = carried;
        --i;
    }
    list[i] = value;

    r->eax = (eax_in & 0xffff00ffu) | ((uint32_t)carried << 8);
    r->ebx = i;
    r->edx = limit;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(i, limit));
}
GP4_IMPL(0x00522cda, FUN_00522cda, "regs(eax:u32, edx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=order_00675958")
