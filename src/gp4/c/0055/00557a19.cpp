#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads an 8-byte record {u16 a, u16 b, i16 c, i16 d} from the stream at esi and advances esi by 8
// (flags of that add) in both modes. When the mode byte 0x0086857c is 1 it also sets 0x00852c38 to
// 0xff and passes its sign-extended value (-1) to 0x00408095 (stores al in 0x006279f8), replaces the
// low words of 0x00870638 / 0x0087063c by a / b with bit 0 cleared, and stores the sign-extended c
// and d in 0x00852c44 / 0x00852c48 (eax ends as the sign-extended d).
// FUNCTION: GP4 0x00557a19
void __cdecl FUN_00557a19(gp4::Regs* r) {
    const uint32_t start = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        GP4_GLOBAL(uint8_t, 0x00852c38) = 0xffu;
        gp4::Regs c = *r;
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00852c38))));
        gp4::call_regs(0x00408095, c);

        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(start));
        GP4_GLOBAL(uint32_t, 0x00870638) = (GP4_GLOBAL(uint32_t, 0x00870638) & 0xffff0000u) | (words[0] & 0xfffeu);
        GP4_GLOBAL(uint32_t, 0x0087063c) = (GP4_GLOBAL(uint32_t, 0x0087063c) & 0xffff0000u) | (words[1] & 0xfffeu);
        GP4_GLOBAL(int32_t, 0x00852c44) = static_cast<int16_t>(words[2]);
        const int32_t last = static_cast<int16_t>(words[3]);
        GP4_GLOBAL(int32_t, 0x00852c48) = last;
        r->eax = static_cast<uint32_t>(last);
    }
    r->esi = start + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(start, 8u));
}
GP4_IMPL(0x00557a19, FUN_00557a19, "regs(eax:u32, esi:ptr[0x10]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
