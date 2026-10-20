#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry k of the dword table at 0x00891bd0 (indexed in a helper so the compiler keeps the table base as the
// only global, as in the original; the loaded dword is overwritten at once).
static __declspec(noinline) uint32_t TableEntry(uint32_t k) {
    return GP4_ARRAY(uint32_t, 0x00891bd0)[k];
}

// Readout set-up for the car/record at esi (sibling of 0x005159e0): byte 0x00893cfb is the complement of the
// bit selected by the record byte +0x7f while the header cell 1 of the table 0x00891bd0 goes to the (empty)
// routine 0x00515b63. Then the record byte +0x7c minus one (masked to 6 bits, clamped to 0x15) selects a row of
// 0x2c bytes at 0x0066d962, offset by the byte 0x006f2c6a[0x0066d936[row]]. The byte 0x006f2c38[+0x7f] (kept in
// edi) is the number of cells: for each one the cursor bytes 0x0088fd94..97 are loaded from 0x00894030[k],
// 0x00894038[k], 0x00894040 and 0x00894041, and the cell is drawn either as a two-digit readout by 0x00510e71
// (when mode byte 0x007c2ea9 is zero, the record byte +0x7f (signed) is at least 2, or the row byte is set) or
// as the character 0x0081739e[k] through the writer 0x0051f3bd, followed by 0x008173a6[k] unless that
// character came back as '0'. al/ah carry the first two cursor bytes (ah survives, al becomes the row byte);
// edx counts the row bytes (the callee's edx is dropped by the pop). Finally bit 7 of byte +0x8a is cleared and
// byte 0x00954169 becomes 0xff. The registers are whatever the last callee leaves (edx, ecx advanced by the
// loop); the flags are those of the final `and`.
// FUNCTION: GP4 0x00515809
void __cdecl FUN_00515809(gp4::Regs* r) {
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    const uint32_t selector = record[0x7f];
    c.eax = selector;
    c.edx = ~(1u << (selector & 31u));
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
    GP4_GLOBAL(uint8_t, 0x00893cfb) = static_cast<uint8_t>(c.edx);
    c.edi = GP4_ARRAY(uint8_t, 0x006f2c38)[selector];

    c.eax = TableEntry(1u);
    gp4::call_regs(0x00515b63, c);
    GP4_GLOBAL(uint8_t, 0x00893cfb) = 0;

    uint32_t row = static_cast<uint8_t>(record[0x7c] - 1u) & 0x3fu;
    if (static_cast<int32_t>(row) > 0x15) {
        row = 0x15u;
    }
    c.eax = row;
    c.ebx = row * 0x2cu;
    const uint32_t group = GP4_ARRAY(uint8_t, 0x0066d936)[row];
    c.edx = GP4_ARRAY(uint8_t, 0x006f2c6a)[group];
    c.ecx = 0;

    do {
        const uint32_t cell = c.edx;
        const uint32_t lo = GP4_ARRAY(uint8_t, 0x00894030)[c.ecx];
        const uint32_t hi = GP4_ARRAY(uint8_t, 0x00894038)[c.ecx];
        const uint32_t xoff = GP4_GLOBAL(uint8_t, 0x00894040);
        const uint32_t yoff = GP4_GLOBAL(uint8_t, 0x00894041);
        GP4_GLOBAL(uint8_t, 0x0088fd94) = static_cast<uint8_t>(lo);
        GP4_GLOBAL(uint8_t, 0x0088fd95) = static_cast<uint8_t>(hi);
        GP4_GLOBAL(uint8_t, 0x0088fd96) = static_cast<uint8_t>(xoff);
        GP4_GLOBAL(uint8_t, 0x0088fd97) = static_cast<uint8_t>(yoff);
        c.edx = cell;
        const uint32_t rowByte = GP4_ARRAY(uint8_t, 0x0066d962)[cell + c.ebx];
        c.eax = (c.eax & 0xffff0000u) | (hi << 8) | rowByte;

        const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        const uint32_t sel = *(reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi)) + 0x7f);
        bool readout = true;
        uint32_t flags = gp4::flags::sub8(mode, 0u);
        if (mode != 0u) {
            flags = gp4::flags::sub8(sel, 2u);
            if (static_cast<int8_t>(sel) < 2) {
                flags = gp4::flags::sub8(rowByte, 0u);
                if (rowByte == 0u) {
                    readout = false;
                }
            }
        }
        c.eflags = gp4::flags::merge(c.eflags, flags);
        if (readout) {
            gp4::call_regs(0x00510e71, c);
        } else {
            c.eax = (c.eax & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x0081739e)[c.ecx];
            gp4::call_regs(0x0051f3bd, c);
            const uint32_t ch = c.eax & 0xffu;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(ch, 0x30u));
            if (ch != 0x30u) {
                c.eax = (c.eax & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x008173a6)[c.ecx];
                gp4::call_regs(0x0051f3bd, c);
            }
        }
        c.edx = cell + 1u;
        c.ecx = c.ecx + 1u;
    } while (static_cast<int32_t>(c.ecx) < static_cast<int32_t>(c.edi));

    uint8_t* const record2 = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    const uint32_t cleared = record2[0x8a] & 0x7fu;
    record2[0x8a] = static_cast<uint8_t>(cleared);
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    *r = c;
}
GP4_IMPL(0x00515809, FUN_00515809, "regs(esi:ptr[0x92]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329+mode_0051f424")
