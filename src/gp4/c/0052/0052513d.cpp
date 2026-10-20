#include <gp4/gp4.h>
#include <gp4/flags.h>

// Car record at byte offset `offset` of the 22-record array 0x0066fef0 (record pointer arithmetic kept out of line
// so the +0x68 / +0x7a fields are record-relative, not extra absolute globals).
static __declspec(noinline) uint8_t* CarRecord(int32_t offset) {
    return GP4_ARRAY(uint8_t, 0x0066fef0) + offset;
}

// pushal wrapper: finds the car behind the selected object. The object's word +0x1a (11 bits) is reduced by
// the word [0x0066d88a] modulo the lap length [0x00868558] into 0..length-1 (the signed result goes to
// [0x00679be8]); then the car-order list 0x0066fe3a (0xffff-terminated byte offsets of car records) is
// walked: records with bit 7 of +0x7a are skipped, a record whose word +0x68 is below the position goes to
// [0x0088fb9c], and the first record whose +0x68 is smaller than the position stops the walk -
// [0x0088fb98] = that record, [0x00679bec] = its list slot. [0x00679bec] otherwise holds the last slot visited.
// popal restores every register; the flags are those of the last compare / test.
// FUNCTION: GP4 0x0052513d
void __cdecl FUN_0052513d(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_ARRAY(uint32_t, 0x006f2f6c)[index]));
    uint32_t ax = GP4_FIELD(uint16_t, object, 0x1a) & 0x7ffu;

    const uint32_t shift = GP4_GLOBAL(uint16_t, 0x0066d88a);
    uint32_t flags = gp4::flags::sub16(ax, shift);
    ax = (ax - shift) & 0xffffu;
    if ((ax & 0x8000u) != 0u) {
        const uint32_t length = GP4_GLOBAL(uint16_t, 0x00868558);
        flags = gp4::flags::add16(ax, length);
        ax = (ax + length) & 0xffffu;
    }
    const uint32_t length = GP4_GLOBAL(uint16_t, 0x00868558);
    flags = gp4::flags::sub16(ax, length);
    if (static_cast<int16_t>(ax) >= static_cast<int16_t>(length)) {
        flags = gp4::flags::sub16(ax, length);
        ax = (ax - length) & 0xffffu;
    }

    GP4_GLOBAL(int32_t, 0x00679be8) = static_cast<int16_t>(ax);
    GP4_GLOBAL(uint32_t, 0x00679bec) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x0088fb98) = 0u;
    GP4_GLOBAL(uint32_t, 0x0088fb9c) = 0u;

    const int16_t* slot = GP4_ARRAY(int16_t, 0x0066fe3a);
    for (;;) {
        GP4_GLOBAL(uint32_t, 0x00679bec) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
        slot += 1;
        const int32_t offset = *slot;
        flags = gp4::flags::sub16(static_cast<uint32_t>(offset), 0xffffu);
        if (static_cast<uint16_t>(offset) == 0xffffu) {
            break;
        }
        uint8_t* record = CarRecord(offset);
        flags = gp4::flags::logic8(record[0x7a] & 0x80u);
        if ((record[0x7a] & 0x80u) != 0u) {
            continue;
        }
        const uint32_t position = GP4_FIELD(uint16_t, record, 0x68);
        flags = gp4::flags::sub16(ax, position);
        if (static_cast<int16_t>(ax) > static_cast<int16_t>(position)) {
            GP4_GLOBAL(uint32_t, 0x00679bec) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
            GP4_GLOBAL(uint32_t, 0x0088fb98) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
            break;
        }
        GP4_GLOBAL(uint32_t, 0x0088fb9c) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052513d, FUN_0052513d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+ptrs_006f2f6c+list_0066fe3c")
