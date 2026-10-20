#include <gp4/gp4.h>

// Group pass over the lap ring (0x160-byte records from 0x008943a0 up to the lap end [0x00866ecc]).
// Records are grouped by their byte +0x2a (a car/lane id); for the first record of each id whose
// 16-byte table entry at 0x006269b8 + id*16 has a record pointer (dword +8), 0x0043d4f7 walks back
// from that pointer to the current record and, when it succeeds, reports a signed run length in
// al and the run's first record in edx: every record from the current one to edx then gets byte
// +0xa9 = +-(0, 1, 2 ... up to |al|) (sign of al), the running count sticking at |al|.
// A second pass walks the ring backwards from the lap end: a record with byte +0x51 bit 7 set
// restarts a countdown at -1 (and a carry flag from its bit 3); the following records get bit 0
// of +0x51 (and bit 3 while the flag is set) while the countdown, [0x006043d4] + |byte +0xa9| of
// the first one, lasts. Every register is preserved (pushal/popal); the flags are those of the final
// `cmp edi, 0x008943a0` (equal).
// FUNCTION: GP4 0x0043d5d8
void __cdecl FUN_0043d5d8(gp4::Regs* r) {
    const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
    uint32_t record = 0x008943a0u;
    uint32_t previous = 0xffffffffu;  // ecx
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;

    // first pass (0x0043d5e3)
    for (;;) {
        const uint32_t id = GP4_FIELD(uint8_t, record, 0x2a);
        if (id == previous) {
            record += 0x160u;
            if (record != lapEnd) {
                continue;
            }
            break;
        }
        previous = id;
        const uint32_t entry = 0x006269b8u + id * 16u;
        const uint32_t start = GP4_FIELD(uint32_t, entry, 8);
        if (start == 0u) {
            continue;
        }

        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = eax;
        call.ebx = entry;
        call.ecx = previous;
        call.edx = edx;
        call.esi = start;
        call.edi = record;
        gp4::call_regs(0x0043d4f7, call);
        eax = call.eax;
        edx = call.edx;
        if ((call.eflags & 1u) != 0u) {
            continue;
        }

        const uint8_t al = static_cast<uint8_t>(eax);
        const bool negative = static_cast<int8_t>(al) < 0;
        const uint8_t limit = negative ? static_cast<uint8_t>(0u - al) : al;  // bl
        uint8_t counter = 0;                                                 // ah
        bool finished = false;
        for (;;) {
            const uint8_t value = negative ? static_cast<uint8_t>(0u - counter) : counter;  // bh
            GP4_FIELD(uint8_t, record, 0xa9) = value;
            if (counter != limit) {
                ++counter;
            }
            if (record == edx) {
                break;
            }
            record += 0x160u;
            if (record == lapEnd) {
                finished = true;
                break;
            }
        }
        eax = (eax & 0xffff00ffu) | (static_cast<uint32_t>(counter) << 8);
        if (finished) {
            break;
        }
    }

    // second pass (0x0043d645): backwards from the lap end down to the first record
    int32_t countdown = 0;  // ebp
    int32_t carry = 0;      // ebx
    uint32_t walker = lapEnd;
    for (;;) {
        walker -= 0x160u;
        const uint8_t flags = GP4_FIELD(uint8_t, walker, 0x51);
        bool mark;
        if ((flags & 0x80u) != 0u) {
            countdown = -1;
            carry = ((flags & 8u) != 0u) ? -1 : 0;
            mark = true;
        } else {
            if (countdown == -1) {
                uint8_t magnitude = GP4_FIELD(uint8_t, walker, 0xa9);
                if (static_cast<int8_t>(magnitude) < 0) {
                    magnitude = static_cast<uint8_t>(0u - magnitude);
                }
                countdown = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006043d4) + magnitude);
            }
            mark = countdown != 0;
            if (mark) {
                --countdown;
            }
        }
        if (mark) {
            GP4_FIELD(uint8_t, walker, 0x51) = static_cast<uint8_t>(GP4_FIELD(uint8_t, walker, 0x51) | 1u);
            if (carry != 0) {
                GP4_FIELD(uint8_t, walker, 0x51) = static_cast<uint8_t>(GP4_FIELD(uint8_t, walker, 0x51) | 8u);
            }
        }
        if (walker == 0x008943a0u) {
            break;
        }
    }

    // cmp edi, 0x8943a0 (equal): ZF and PF set
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043d5d8, FUN_0043d5d8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043ec1a+ring_008943a0+surf_0041f02b+tab_0043d5d8")
