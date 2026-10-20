#include <gp4/gp4.h>
#include <gp4/flags.h>

enum : uint32_t { kRecord = 0x160u };

static inline uint32_t g32(uint32_t addr) { return GP4_GLOBAL(uint32_t, addr); }

// The 16-byte entries at 0x006269b8 (word speed, word clamped engine figure, dword segment
// [0x00603ed0], dword [0x00602d88]); reached through ebx in the original.
static __declspec(noinline) uint8_t* keyEntry(uint32_t index) {
    return GP4_ARRAY(uint8_t, 0x006269b8) + index * 0x10u;
}

static __declspec(noinline) void clearFirstEntry(uint8_t* entry) {
    GP4_FIELD(uint16_t, entry, 0) = 0;
    GP4_FIELD(uint16_t, entry, 4) = 0;
    GP4_FIELD(uint16_t, entry, 6) = 0;
    GP4_FIELD(uint32_t, entry, 8) = 0;
    GP4_FIELD(uint32_t, entry, 0xc) = 0;
    GP4_FIELD(uint16_t, entry, 2) = 0x7700;
}

// Sum of the record words +0 and +0x18 of the next record minus those of this one (16-bit).
static inline uint16_t gradient(uint32_t record) {
    uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
    return static_cast<uint16_t>(GP4_FIELD(uint16_t, p, 0x178) + GP4_FIELD(uint16_t, p, 0x160)
                                 - GP4_FIELD(uint16_t, p, 0x18) - GP4_FIELD(uint16_t, p, 0));
}

// The 0x004056c9 measurement scaled down: (eax >> 8) clamped to 0x5800.
static inline int32_t engineFigure(uint32_t value) {
    int32_t figure = static_cast<int32_t>(value) >> 8;
    if (figure > 0x5800) {
        figure = 0x5800;
    }
    return figure;
}

// One record forward along the ring: with the direction byte 0x0086857a set it only stops below
// the pit end [0x00866ef8]; otherwise the lap end [0x00866ecc] / pit start [0x00891500] / wrap
// [0x00866ec4] / pit end rules.
static inline uint32_t stepForward(uint32_t record, uint8_t direction) {
    record += kRecord;
    if (direction != 0u) {
        if (record >= g32(0x00866ef8)) {
            record -= kRecord;
        }
    } else if (record >= g32(0x00866ecc)) {
        if (record >= g32(0x00891500)) {
            if (record >= g32(0x00866ef8)) {
                record = g32(0x00866ef8) - kRecord;
            }
        } else {
            record -= g32(0x00866ec4);
        }
    }
    return record;
}

// Mode 0x0086857a != 0: measure the run of equal gradients from the record after edi. Returns
// false (registers as the original leaves them) when the run would start at the pit end.
static inline bool measureRun(gp4::Regs& s) {
    GP4_GLOBAL(int32_t, 0x00626964) = 0;
    GP4_GLOBAL(int32_t, 0x00626968) = 0;
    GP4_GLOBAL(int32_t, 0x0062695c) = 0;
    GP4_GLOBAL(int32_t, 0x00626960) = 0;

    uint32_t record = s.edi + kRecord;
    const uint32_t limit = g32(0x00866ef8) - kRecord;
    s.edx = limit;
    if (record >= limit) {
        return false;
    }
    GP4_GLOBAL(int32_t, 0x0062ac48) = 2;
    const uint16_t run = gradient(record);
    for (;;) {
        record += kRecord;
        if (record >= limit) {
            break;
        }
        if (gradient(record) != run) {
            break;
        }
        GP4_GLOBAL(int32_t, 0x0062ac48) += 1;
    }

    int32_t value = static_cast<int16_t>(run);
    GP4_GLOBAL(int32_t, 0x0062ac68) = 0;
    if (value != 0) {
        const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bec));
        const uint32_t low = static_cast<uint32_t>(product);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        const int32_t divisor = static_cast<int32_t>((low >> 14) | (high << 18));
        GP4_GLOBAL(int32_t, 0x0062ac64) = 0x1000000 / divisor;
    } else {
        GP4_GLOBAL(int32_t, 0x0062ac64) = 0;
    }
    if (value < 0) {
        value = -value;
    }
    GP4_GLOBAL(int32_t, 0x00626964) = value;
    GP4_GLOBAL(int32_t, 0x00626968) = value;

    const int32_t difference = GP4_GLOBAL(int32_t, 0x00626968) - GP4_GLOBAL(int32_t, 0x00626964);
    int32_t count = GP4_GLOBAL(int32_t, 0x0062ac48);
    if (count == 0) {
        count = 1;
    }
    const int32_t slope = difference / count;
    s.edx = static_cast<uint32_t>(difference % count);
    GP4_GLOBAL(int32_t, 0x0062696c) = slope;
    s.ecx = g32(0x00626964);
    GP4_GLOBAL(int32_t, 0x0062695c) = GP4_GLOBAL(int32_t, 0x00626964);
    GP4_GLOBAL(int32_t, 0x00626960) = GP4_GLOBAL(int32_t, 0x00626968);
    if (GP4_GLOBAL(int32_t, 0x0062ac64) != 0 && GP4_GLOBAL(int32_t, 0x0062ac64) < 0) {
        GP4_GLOBAL(uint32_t, 0x0062695c) = 0u - g32(0x0062695c);
        GP4_GLOBAL(uint32_t, 0x00626960) = 0u - g32(0x00626960);
    }
    s.eax = static_cast<uint32_t>(slope);
    return true;
}

// Track-wide pass for car 0 (0x0044109d resets car record 0 and leaves it in esi). Starting at the
// pit start (byte 0x0086857a set) or the lap start, it walks the segment ring run by run. With
// 0x0086857a clear each run is the next key-frame record of the stream [0x00866edc] (0x0043df22:
// count [0x0062ac48], speeds 0x00626964 / 0x00626968; a zero header ends the pass); otherwise a
// run is the stretch of records with equal gradients of words +0/+0x18, whose speed is
// 0x1000000 / ((gradient * [0x006f2bec]) >> 14). Slow runs (speed < 0x3c, or short with
// [0x0062698c] set by 0x0043eca1) only lower the level 0x00626950 by [0x0062694c] per record and
// step over them; the others emit entry ([0x0062698a] & 0xff) of the 16-byte table at 0x006269b8
// (signed speed, engine figures from 0x004056c9, segment, [0x00602d88]), run the per-run updates
// and the backward braking walk 0x0043e0bc, and continue from the run end [0x00626994]. The
// flags left are those of the final `cmp word [0x0062698a], 0x60`.
// FUNCTION: GP4 0x0043ff93
void __cdecl FUN_0043ff93(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00622426) = 0;
    GP4_GLOBAL(uint32_t, 0x00622434) = 0x14;
    gp4::call_regs(0x0044109d, s);
    GP4_GLOBAL(uint32_t, 0x00626980) = s.esi;
    GP4_GLOBAL(int32_t, 0x00626954) = 0;
    GP4_GLOBAL(int32_t, 0x00626958) = 0;
    GP4_GLOBAL(int32_t, 0x0062695c) = 0;
    GP4_GLOBAL(int32_t, 0x00626960) = 0;

    uint8_t* const first = keyEntry(0);
    if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
        s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
        s.edi = g32(0x00891500);
    } else {
        s.edi = g32(0x00891504);
        s.esi = g32(0x00866edc);
        GP4_GLOBAL(uint32_t, 0x00626984) = s.esi;
        s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
        GP4_GLOBAL(uint16_t, 0x0062698a) = 0;
        clearFirstEntry(first);
        GP4_GLOBAL(int32_t, 0x00626964) = 0;
        GP4_GLOBAL(int32_t, 0x00626968) = 0;
        GP4_GLOBAL(int32_t, 0x00626950) = 0;
    }

    for (;;) {
        s.esi = g32(0x00626984);
        GP4_GLOBAL(uint32_t, 0x00626990) = s.edi;
        GP4_GLOBAL(uint32_t, 0x00626954) = g32(0x0062695c);
        s.eax = g32(0x00626960);
        GP4_GLOBAL(uint32_t, 0x00626958) = s.eax;
        GP4_GLOBAL(uint32_t, 0x00603ed0) = 0;

        if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
            if (!measureRun(s)) {
                break;
            }
        } else {
            gp4::call_regs(0x0043df22, s);
            if ((s.eflags & gp4::flags::CF) == 0u) {
                break;
            }
            gp4::call_regs(0x0043dd13, s);
        }
        gp4::call_regs(0x0043eca1, s);

        s.ecx = g32(0x00626964);
        s.eax = g32(0x0062ac48);
        GP4_GLOBAL(uint32_t, 0x0062ac44) = s.eax;
        if (static_cast<int32_t>(s.ecx) < 0x3c
            || (GP4_GLOBAL(int32_t, 0x0062ac48) < 4 && GP4_GLOBAL(uint8_t, 0x0062698c) != 0u)) {
            // Too slow to brake for: lower the level per record and step over the run.
            do {
                int32_t level = GP4_GLOBAL(int32_t, 0x00626950) - GP4_GLOBAL(int32_t, 0x0062694c);
                if (level < GP4_GLOBAL(int32_t, 0x00626968)) {
                    level = GP4_GLOBAL(int32_t, 0x00626968);
                }
                GP4_GLOBAL(int32_t, 0x00626950) = level;
                s.eax = static_cast<uint32_t>(level);
                s.edi = stepForward(s.edi, GP4_GLOBAL(uint8_t, 0x0086857a));
                GP4_GLOBAL(uint32_t, 0x0062ac44) -= 1u;
            } while (GP4_GLOBAL(uint32_t, 0x0062ac44) != 0u);
            gp4::call_regs(0x0043ec1a, s);
            GP4_GLOBAL(uint32_t, 0x00626984) = s.esi;
            continue;
        }

        GP4_GLOBAL(uint32_t, 0x00626984) = s.esi;
        s.esi = g32(0x00626980);
        GP4_GLOBAL(uint16_t, 0x0062698a) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0062698a) + 1u);

        const int64_t drop = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062ac48))
                           * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062694c));
        s.edx = static_cast<uint32_t>(static_cast<uint64_t>(drop) >> 32);
        int32_t level = static_cast<int32_t>(g32(0x00626950) - static_cast<uint32_t>(drop));
        if (level < GP4_GLOBAL(int32_t, 0x00626968)) {
            level = GP4_GLOBAL(int32_t, 0x00626968);
        }
        GP4_GLOBAL(int32_t, 0x00626950) = level;
        s.eax = static_cast<uint32_t>(level);
        gp4::call_regs(0x0043eb10, s);
        gp4::call_regs(0x0043ea29, s);

        const uint32_t index = GP4_GLOBAL(uint16_t, 0x0062698a) & 0xffu;
        uint8_t* const entry = keyEntry(index);
        s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        uint32_t speed = g32(0x00626964);
        if (GP4_GLOBAL(int32_t, 0x0062ac64) < 0) {
            speed = 0u - speed;
        }
        if (GP4_GLOBAL(uint8_t, 0x00622426) == 0u) {
            GP4_FIELD(uint16_t, entry, 0) = static_cast<uint16_t>(speed);
        }

        s.eax = g32(0x00626964);
        GP4_GLOBAL(uint32_t, 0x00602d7c) = s.eax;
        gp4::call_regs(0x004056c9, s);
        int32_t figure = engineFigure(s.eax);
        GP4_GLOBAL(int32_t, 0x00626974) = figure;
        s.eax = static_cast<uint32_t>(figure);
        if (GP4_GLOBAL(uint8_t, 0x00622426) == 0u) {
            GP4_FIELD(uint16_t, entry, 2) = static_cast<uint16_t>(figure);
        }
        s.eax = g32(0x00602d88);
        GP4_FIELD(uint32_t, entry, 0xc) = s.eax;

        s.eax = g32(0x00626968);
        GP4_GLOBAL(uint32_t, 0x00602d7c) = s.eax;
        gp4::call_regs(0x004056c9, s);
        figure = engineFigure(s.eax);
        GP4_GLOBAL(int32_t, 0x00626970) = figure;
        s.eax = static_cast<uint32_t>(figure);
        gp4::call_regs(0x0043ec1a, s);

        const int32_t change = GP4_GLOBAL(int32_t, 0x00626970) - GP4_GLOBAL(int32_t, 0x00626974);
        int32_t count = GP4_GLOBAL(int32_t, 0x0062ac48);
        if (count == 0) {
            count = 1;
        }
        s.ebp = static_cast<uint32_t>(count);
        GP4_GLOBAL(int32_t, 0x00626978) = change / count;
        s.edx = static_cast<uint32_t>(change % count);
        s.eax = g32(0x00603ed0);
        GP4_FIELD(uint32_t, entry, 8) = s.eax;

        if (GP4_GLOBAL(uint8_t, 0x0086857a) == 0u) {
            gp4::call_regs(0x0043dda2, s);
        }
        gp4::call_regs(0x0043e237, s);

        s.eax = g32(0x0062ac48) * kRecord;
        uint32_t end = g32(0x00626990);
        if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
            end += s.eax;
            if (end >= g32(0x00866ef8)) {
                end = g32(0x00866ef8) - kRecord;
            }
        } else if (end >= g32(0x00891500)) {
            end += s.eax;
            if (end >= g32(0x00866ef8)) {
                end = g32(0x00866ef8) - kRecord;
            }
        } else {
            end += s.eax;
            if (end >= g32(0x00866ecc)) {
                end -= g32(0x00866ec4);
            }
        }
        s.edx = end;
        GP4_GLOBAL(uint32_t, 0x00626994) = end;

        s.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00626988)));
        GP4_FIELD(uint16_t, s.esi, 0x9a) = static_cast<uint16_t>(s.eax);

        if (GP4_GLOBAL(uint8_t, 0x0062698c) != 0xffu) {
            s.edi = g32(0x00626990);
            gp4::call_regs(0x0043e059, s);
        }
        s.edi = g32(0x00626990);
        gp4::call_regs(0x0043e0bc, s);
        s.edi = g32(0x00626994);
    }

    *r = s;
    r->eflags = gp4::flags::merge(s.eflags, gp4::flags::sub16(GP4_GLOBAL(uint16_t, 0x0062698a), 0x60u));
}
GP4_IMPL(0x0043ff93, FUN_0043ff93, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); wall=1800; globals=fuzz:i32+layout=cars_0066fef0+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0040fbf3+wheel_00410a1c+state_0043c1ba+state_00404ed0+state_00404f33+engine_00413606+drive_004054e6+lanes_00429fcd+bank_004279ab+ring_008943a0+state_00405489+drive_00405552+search_00405552+seg_00405552+ring_0043faf5+tab_00425f41+rec_0043d38f+ptr_00603ed0+state_0043e0bc+list_0066fe3c+run_0043ff93")
