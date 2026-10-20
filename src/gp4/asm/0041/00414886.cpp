#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic op (AND/OR): CF = OF = AF = 0.
static inline uint32_t flagsLogic8(uint8_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80u);
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

static inline uint16_t neg16(uint16_t v) {
    return static_cast<uint16_t>(0u - v);
}

// esi = car record, edi = pointer to a word (angle) compared with the word at esi+0xe. The tail
// clears or sets the steering-state bits 8 / 4 of the bytes at +0x163 / +0x6f; the flags are those
// of the last byte operation on +0x6f.
// FUNCTION: GP4 0x00414886
void __cdecl FUN_00414886(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    enum { Recover, Set, Clear };
    int target;

    if (GP4_FIELD(int16_t, rec, 0x9a) < 0x1d5) {
        target = Recover;
    } else {
        uint16_t bp = GP4_FIELD(uint16_t, rec, 0x90);
        uint16_t dx = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x0e) - *reinterpret_cast<const uint16_t*>(other));
        if (static_cast<int16_t>(bp) < 0) dx = neg16(dx);
        uint16_t ax = dx;
        if (static_cast<int16_t>(ax) < 0) ax = neg16(ax);
        if (static_cast<int16_t>(ax) >= 0x4000) {
            bp = neg16(bp);
            ax = static_cast<uint16_t>(neg16(ax) + 0x8000u);
            if (static_cast<int16_t>(dx) < 0) ax = neg16(ax);
            dx = ax;
        }
        eax = setLow16(eax, ax);
        edx = setLow16(edx, dx);

        const int16_t angle = static_cast<int16_t>(dx);
        if (!(rec[0x163] & 0x10u)) {
            if (angle < GP4_GLOBAL(int16_t, 0x00603bc2)) {
                target = (angle > GP4_GLOBAL(int16_t, 0x00603bc4)) ? Clear : Set;
            } else {
                bp = neg16(bp);
                target = Set;
            }
        } else {
            if (angle < GP4_GLOBAL(int16_t, 0x00603bc6)) {
                target = (angle < GP4_GLOBAL(int16_t, 0x00603bc8)) ? Set : Recover;
            } else {
                bp = neg16(bp);
                target = Set;
            }
        }
        ebp = setLow16(ebp, bp);
    }

    if (target == Recover) {
        gp4::Regs call = *r;
        call.eax = eax;
        call.edx = edx;
        call.ebp = ebp;
        gp4::call_regs(0x00418097, call);

        target = Clear;
        if (GP4_FIELD(int16_t, rec, 0x9a) < 0x1255) {
            uint16_t ax = GP4_FIELD(uint16_t, rec, 0x90);
            if (static_cast<int16_t>(ax) < 0) ax = neg16(ax);
            ax = static_cast<uint16_t>(ax - 0x200u);
            eax = setLow16(eax, ax);
            if (!(static_cast<int16_t>(ax) > GP4_FIELD(int16_t, rec, 0xb0))) {
                ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x90) - GP4_FIELD(uint16_t, rec, 0x94));
                if (static_cast<int16_t>(ax) < 0) ax = neg16(ax);
                eax = setLow16(eax, ax);
                if (!(static_cast<int16_t>(ax) < 0x300)) {
                    ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x94) ^ GP4_FIELD(uint16_t, rec, 0x90));
                    eax = setLow16(eax, ax);
                    if (static_cast<int16_t>(ax) < 0) rec[0x6f] |= 2u;
                }
            }
        }
    }

    if (target == Set) {
        rec[0x163] |= 8u;
        rec[0x6f] |= (static_cast<int16_t>(ebp) < 0) ? 4u : 8u;
    } else {
        rec[0x163] &= 0xf7u;
        rec[0x6f] &= 0xf7u;
        rec[0x6f] &= 0xfbu;
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flagsLogic8(rec[0x6f]);
}
GP4_IMPL(0x00414886, FUN_00414886, "regs(esi:ptr[0x16b]:bytes, edi:ptr[8]:bytes, eax:u32, edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
