#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* rec_at(uint32_t addr) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(addr));
}

// Ring-record fix-up after a pit-lane splice. Copies the lap/pit fields of the record [0x00866f00]
// (word +0, +0x16, +0x18, byte +0x28, +0x29) to the record [0x00891500], copies byte +0xa6 of
// [0x00866f04] to [0x00866ef8], clears bits 0x30 of byte +0x48 of the record before [0x00891500]
// (-0x160) and of [0x00866ef8], and copies words +0x60/+0x62 from [0x00891500] to the record before it
// and from the record before [0x00866ef8] to [0x00866ef8]. Leaves al = byte +0xa6 of [0x00866f04],
// ebx = [0x00866ef8], edi = [0x00866ef8] - 0x160 and the flags of that last `sub edi, 0x160`.
// FUNCTION: GP4 0x00559591
void __cdecl FUN_00559591(gp4::Regs* r) {
    uint8_t* src = rec_at(GP4_GLOBAL(uint32_t, 0x00866f00));
    uint8_t* dst = rec_at(GP4_GLOBAL(uint32_t, 0x00891500));
    GP4_FIELD(uint16_t, dst, 0x00) = GP4_FIELD(uint16_t, src, 0x00);
    GP4_FIELD(uint16_t, dst, 0x16) = GP4_FIELD(uint16_t, src, 0x16);
    GP4_FIELD(uint16_t, dst, 0x18) = GP4_FIELD(uint16_t, src, 0x18);
    GP4_FIELD(uint8_t, dst, 0x28) = GP4_FIELD(uint8_t, src, 0x28);
    GP4_FIELD(uint8_t, dst, 0x29) = GP4_FIELD(uint8_t, src, 0x29);

    uint8_t* exit_rec = rec_at(GP4_GLOBAL(uint32_t, 0x00866f04));
    uint8_t* pit_end = rec_at(GP4_GLOBAL(uint32_t, 0x00866ef8));
    const uint8_t lane = GP4_FIELD(uint8_t, exit_rec, 0xa6);
    GP4_FIELD(uint8_t, pit_end, 0xa6) = lane;

    uint8_t* pit_start_prev = rec_at(GP4_GLOBAL(uint32_t, 0x00891500) - 0x160u);
    GP4_FIELD(uint8_t, pit_start_prev, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, pit_start_prev, 0x48) & 0xcfu);
    pit_end = rec_at(GP4_GLOBAL(uint32_t, 0x00866ef8));
    GP4_FIELD(uint8_t, pit_end, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, pit_end, 0x48) & 0xcfu);

    uint8_t* d1 = rec_at(GP4_GLOBAL(uint32_t, 0x00891500) - 0x160u);
    uint8_t* s1 = rec_at(GP4_GLOBAL(uint32_t, 0x00891500));
    GP4_FIELD(uint16_t, d1, 0x60) = GP4_FIELD(uint16_t, s1, 0x60);
    GP4_FIELD(uint16_t, d1, 0x62) = GP4_FIELD(uint16_t, s1, 0x62);

    const uint32_t end_addr = GP4_GLOBAL(uint32_t, 0x00866ef8);
    uint8_t* d2 = rec_at(end_addr);
    const uint32_t prev_addr = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
    uint8_t* s2 = rec_at(prev_addr);
    GP4_FIELD(uint16_t, d2, 0x60) = GP4_FIELD(uint16_t, s2, 0x60);
    GP4_FIELD(uint16_t, d2, 0x62) = GP4_FIELD(uint16_t, s2, 0x62);

    r->eax = (r->eax & 0xffffff00u) | lane;
    r->ebx = end_addr;
    r->edi = prev_addr;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00866ef8), 0x160u));
}
GP4_IMPL(0x00559591, FUN_00559591, "regs(eax:u32) -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pit_0055a9f8+copy_00559571")
