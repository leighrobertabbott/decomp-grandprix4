#include <gp4/gp4.h>

// pushal ... popal routine (caller 0x0040d4c4). Rebuilds the 8-byte rows of the table at 0x00604435
// (cleared first, 0x3390 bytes). A record's row is 0x00604435 + 8 * word[record + 0x7a]; words +0/+2
// of the row get a lap position (word +0x1a & 0x7ff, minus the lap length [0x00868558] when it is not
// below it, signed 16-bit) and a sub-position:
//   - the first three pit records ([0x00891500] on) take the positions of the three records from the
//     pit entry [0x00866f00] on (sub 0);
//   - pit records from the fourth one up to [0x00866ed0], and from [0x00866ed4] while three more records
//     remain before the pit end [0x00866ef8], take the position of the record 0x0040d5ef finds for
//     them (edx) and its result (ax);
//   - the last three pit records take the positions of the three records before the pit exit
//     [0x00866f04] (sub = the last 0x0040d5ef result);
//   - the records of the window [0x00866ed0, 0x00866ed4) are interpolated linearly in 18.14 fixed
//     point between the row before the window and the row of its end (+ lap length when that one
//     lies before the start);
//   - every lap record 0x008943a0 .. [0x00866ecc] takes its own position (sub 0).
// 0x0040d9e6 then fills words +4/+6 and byte 0x00604430 becomes 0xff. Every register is restored;
// the flags are those 0x0040d9e6 leaves.
static inline uint8_t* ptrOf(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline uint32_t rowOf(uint32_t record) {
    return static_cast<uint32_t>(GP4_FIELD(uint16_t, ptrOf(record), 0x7a)) * 8u + 0x00604435u;
}

static inline uint16_t reduced(uint16_t position) {
    const int16_t length = GP4_GLOBAL(int16_t, 0x00868558);
    if (static_cast<int16_t>(position) >= length) {
        position = static_cast<uint16_t>(position - static_cast<uint16_t>(length));
    }
    return position;
}

static inline uint16_t positionOf(uint32_t record) {
    return reduced(static_cast<uint16_t>(GP4_FIELD(uint16_t, ptrOf(record), 0x1a) & 0x7ffu));
}

static inline void storeRow(uint32_t row, uint16_t position, uint16_t sub) {
    GP4_FIELD(uint16_t, ptrOf(row), 0) = position;
    GP4_FIELD(uint16_t, ptrOf(row), 2) = sub;
}

static inline uint32_t fixedOf(uint32_t row) {
    return (static_cast<uint32_t>(GP4_FIELD(uint16_t, ptrOf(row), 0)) << 14)
        + static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, ptrOf(row), 2)));
}

// FUNCTION: GP4 0x0040d644
void __cdecl FUN_0040d644(gp4::Regs* r) {
    // rep stosb (direction flag clear)
    uint8_t* table = GP4_ARRAY(uint8_t, 0x00604435);
    for (uint32_t i = 0; i < 0x3390u; ++i) {
        table[i] = 0u;
    }

    uint32_t pit = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t lap = GP4_GLOBAL(uint32_t, 0x00866f00);
    for (int k = 0; k < 3; ++k) {
        storeRow(rowOf(pit), positionOf(lap), 0u);
        pit += 0x160u;
        lap += 0x160u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    pit = GP4_GLOBAL(uint32_t, 0x00891500) + 0x420u;
    do {
        c.edi = pit;
        gp4::call_regs(0x0040d5ef, c);
        storeRow(rowOf(pit), positionOf(c.edx), static_cast<uint16_t>(c.eax));
        pit += 0x160u;
    } while (pit < GP4_GLOBAL(uint32_t, 0x00866ed0));

    pit = GP4_GLOBAL(uint32_t, 0x00866ed4);
    do {
        c.edi = pit;
        gp4::call_regs(0x0040d5ef, c);
        storeRow(rowOf(pit), positionOf(c.edx), static_cast<uint16_t>(c.eax));
        pit += 0x160u;
    } while (pit + 0x420u < GP4_GLOBAL(uint32_t, 0x00866ef8));
    const uint16_t sub = static_cast<uint16_t>(c.eax);

    pit = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x420u;
    lap = GP4_GLOBAL(uint32_t, 0x00866f04) - 0x420u;
    for (int k = 0; k < 3; ++k) {
        storeRow(rowOf(pit), positionOf(lap), sub);
        pit += 0x160u;
        lap += 0x160u;
    }

    const uint32_t first = GP4_GLOBAL(uint32_t, 0x00866ed0);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00866ed4);
    const int32_t count = static_cast<int32_t>(last - first + 0x160u) / 0x160;
    uint32_t position = fixedOf(rowOf(first - 0x160u));
    uint32_t target = fixedOf(rowOf(last));
    if (static_cast<int32_t>(target) < static_cast<int32_t>(position)) {
        target += static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558)) << 14;
    }
    const int32_t step = static_cast<int32_t>(target - position) / count;

    uint32_t record = first;
    do {
        position += static_cast<uint32_t>(step);
        storeRow(rowOf(record), reduced(static_cast<uint16_t>(position >> 14)),
                 static_cast<uint16_t>(position & 0x3fffu));
        record += 0x160u;
    } while (record != last);

    uint32_t lapRecord = 0x008943a0u;
    uint32_t lastRow;
    uint16_t lastPosition;
    do {
        lastRow = rowOf(lapRecord);
        lastPosition = positionOf(lapRecord);
        storeRow(lastRow, lastPosition, 0u);
        lapRecord += 0x160u;
    } while (lapRecord != GP4_GLOBAL(uint32_t, 0x00866ecc));

    // registers as the original holds them at the call (cmp edi, [0x866ecc] equal: ZF, PF)
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.eax = position & 0x3fffu;
    s.ebx = last;
    s.ecx = lastRow;
    s.edx = ((position >> 14) & 0xffff0000u) | lastPosition;
    s.esi = position;
    s.edi = lapRecord;
    s.ebp = static_cast<uint32_t>(step);
    s.eflags = (r->eflags & ~0x8d5u) | 0x44u;
    gp4::call_regs(0x0040d9e6, s);
    GP4_GLOBAL(uint8_t, 0x00604430) = 0xffu;

    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0040d644, FUN_0040d644, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+pit_00406dbb+pitgeo_0040d644")
