#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte-code walker over the string at esi (same opcode classes as 0x0054d0e4: >= 0x14, 8 and 9 are one
// byte, 5 is two bytes, 3 is three bytes, every other opcode < 0x14 ends the walk). A first "other"
// opcode takes the length table 0x00833f98 instead (ebx = &table[opcode], the walk ends with the flags
// of `add esi, [ebx]`). Each opcode 3 sets edx to the big-endian word of its two operand bytes and calls
// the empty routine 0x00531b34, then raises eax (0 at entry) to ecx if ecx is above it. esi, edx, ecx
// and ebp are restored on exit (push/pop), so only eax, ebx (table path) and the flags are results;
// the flags are those of the last `cmp byte [esi], 5` (or of the table `add`).
// FUNCTION: GP4 0x0054d133
void __cdecl FUN_0054d133(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t eax = 0u;
    uint8_t op = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));

    if (op < 0x14u && op != 8u && op != 9u && op != 3u && op != 5u) {
        const uint32_t delta = GP4_ARRAY(uint32_t, 0x00833f98)[op];
        r->ebx = 0x00833f98u + (static_cast<uint32_t>(op) << 2);
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, delta));
        return;
    }

    for (;;) {
        if (op >= 0x14u || op == 8u || op == 9u) {
            esi += 1u;
        } else if (op == 3u) {
            gp4::cdecl_<0x00531b34, void>();
            if (!(r->ecx <= eax)) {
                eax = r->ecx;
            }
            esi += 3u;
        } else if (op == 5u) {
            esi += 2u;
        } else {
            break;
        }
        op = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
    }

    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(op, 5u));
}
GP4_IMPL(0x0054d133, FUN_0054d133, "regs(ebx:u32, ecx:u32{0,1,2,0x10,0x7fffffff,0xffffffff}, esi:ptr[0x40]:u8{0,1,2,3,4,5,6,7,8,9,0x0a,0x13,0x14,0x20,0xff}) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
