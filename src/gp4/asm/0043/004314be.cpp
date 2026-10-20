#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t v) {
    uint32_t b = v & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;
}

// Flags of an 8-bit AND/OR result (CF = OF = AF = 0).
static inline uint32_t LogicFlags8(uint8_t v) {
    return ParityFlag(v) | (v == 0u ? 0x40u : 0u) | ((v & 0x80u) ? 0x80u : 0u);
}

static inline void SetLo16(uint32_t& reg, uint16_t v) { reg = (reg & 0xffff0000u) | v; }

// FUNCTION: GP4 0x004314be
// esi is a car record (stride 0x41c at 0x0066fef0). Unless byte +0xe5 bit 2 is set, it stores
//   [0x934df8] = max(word +0x9a, [0x6013e6]), [0x934df0] = min(that, [0x6079e8]), [0x934df4] = that >> 10
// and walks the car-order list 0x0066fe3c backwards from entry +0x7d - 2 (stops at a negative word).
// Cars with +0x7a bit 7 are skipped; a car behind (+0x68 difference < 0) or further ahead than
// [0x934df4] ends the walk. While word +0xe0 is zero and the car is closer than [0x603b0e], the gap
// (+0xe2 difference or 0x0040dc8a) feeds 0x0042ac43 (the inline copy for +0xe5 bit 2 cannot run: that
// bit made the function exit at entry). A car laterally inside |+0x90| < (+0x11c/+0x11e by side) + 0x400
// (0x280 when its +0x8d bit 1 is set), slower ([0x934df8] - +0x9a > 0), on the same +0x8d bit 1 side, with
// gap >= [0x6079e4] and closing*[0x6079e6] >= gap*[0x934df0] sets bit 4 of byte +0xff; otherwise the bit
// is cleared. Flags are those of that final AND/OR; eax/ebx/ecx/edx keep the walk's working values.
void __cdecl FUN_004314be(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    bool hit = false;

    if ((GP4_FIELD(uint8_t, self, 0xe5) & 4u) == 0u) {
        uint16_t speed = GP4_FIELD(uint16_t, self, 0x9a);
        if ((int16_t)speed < GP4_GLOBAL(int16_t, 0x006013e6)) {
            speed = GP4_GLOBAL(uint16_t, 0x006013e6);
        }
        uint16_t capped = speed;
        if ((int16_t)capped > GP4_GLOBAL(int16_t, 0x006079e8)) {
            capped = GP4_GLOBAL(uint16_t, 0x006079e8);
        }
        SetLo16(edx, capped);
        const uint16_t reach = (uint16_t)((int16_t)speed >> 10);
        SetLo16(ecx, reach);
        GP4_GLOBAL(uint16_t, 0x00934df0) = capped;
        GP4_GLOBAL(uint16_t, 0x00934df4) = reach;
        GP4_GLOBAL(uint16_t, 0x00934df8) = speed;

        eax = GP4_FIELD(uint8_t, self, 0x7d);
        uint32_t entry = 0x0066fe3cu + eax;
        for (;;) {
            entry -= 2u;
            const uint16_t offset = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(entry));
            SetLo16(eax, offset);
            if ((int16_t)offset < 0) break;
            ebx = (uint32_t)(int32_t)(int16_t)offset + 0x0066fef0u;
            uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(ebx));
            if ((GP4_FIELD(uint8_t, other, 0x7a) & 0x80u) != 0u) continue;

            const uint16_t ahead = (uint16_t)(GP4_FIELD(uint16_t, other, 0x68) - GP4_FIELD(uint16_t, self, 0x68));
            SetLo16(eax, ahead);
            if ((int16_t)ahead < 0) break;

            if (GP4_FIELD(uint16_t, self, 0xe0) == 0u && (int16_t)ahead < GP4_GLOBAL(int16_t, 0x00603b0e)) {
                const uint32_t saved = eax;
                if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, other, 0x9c) & 2u) == 0u) {
                    SetLo16(eax, (uint16_t)(GP4_FIELD(uint16_t, other, 0xe2) - GP4_FIELD(uint16_t, self, 0xe2)));
                } else {
                    gp4::Regs call = *r;
                    call.eax = eax; call.ebx = ebx; call.ecx = ecx; call.edx = edx; call.edi = entry;
                    gp4::call_regs(0x0040dc8a, call);
                    eax = call.eax; ecx = call.ecx; edx = call.edx;
                }
                if ((GP4_FIELD(uint8_t, self, 0xe5) & 4u) == 0u) {
                    gp4::Regs call = *r;
                    call.eax = eax; call.ebx = ebx; call.ecx = ecx; call.edx = edx; call.edi = entry;
                    gp4::call_regs(0x0042ac43, call);
                    eax = call.eax; ecx = call.ecx; edx = call.edx;
                } else {
                    uint16_t lateral = (uint16_t)(GP4_FIELD(uint16_t, self, 0x90) - GP4_FIELD(uint16_t, other, 0x90));
                    if ((int16_t)lateral < 0) lateral = (uint16_t)(0u - lateral);
                    SetLo16(edx, lateral);
                    if ((int16_t)lateral < GP4_GLOBAL(int16_t, 0x00603aec)) {
                        uint16_t gain = (uint16_t)(GP4_GLOBAL(uint16_t, 0x00603af6) - (uint16_t)eax);
                        SetLo16(ecx, gain);
                        if ((int16_t)gain >= 0) {
                            if ((int16_t)gain > GP4_GLOBAL(int16_t, 0x00603af8)) {
                                gain = GP4_GLOBAL(uint16_t, 0x00603af8);
                                SetLo16(ecx, gain);
                            }
                            const int64_t first = (int64_t)(int32_t)(int16_t)gain * (int64_t)GP4_GLOBAL(int32_t, 0x00601c68);
                            const int32_t scaled = (int32_t)(uint32_t)((uint64_t)first >> 14);
                            const int64_t second = (int64_t)scaled * (int64_t)GP4_GLOBAL(int32_t, 0x00603854);
                            GP4_FIELD(uint16_t, self, 0xe0) = (uint16_t)((uint64_t)second >> 16);
                            edx = (uint32_t)((uint64_t)second >> 32);
                        }
                    }
                }
                eax = saved;
            }

            if ((int16_t)(uint16_t)eax > GP4_GLOBAL(int16_t, 0x00934df4)) break;

            uint16_t side = GP4_FIELD(uint16_t, other, 0x90);
            uint16_t room = ((int16_t)side < 0) ? GP4_FIELD(uint16_t, self, 0x11c) : GP4_FIELD(uint16_t, self, 0x11e);
            if ((int16_t)side < 0) side = (uint16_t)(0u - side);
            SetLo16(ecx, side);
            room = (uint16_t)(room + 0x400u);
            if ((GP4_FIELD(uint8_t, other, 0x8d) & 2u) != 0u) room = 0x280u;
            SetLo16(edx, room);
            if ((int16_t)side >= (int16_t)room) continue;

            const uint16_t closing = (uint16_t)(GP4_GLOBAL(uint16_t, 0x00934df8) - GP4_FIELD(uint16_t, other, 0x9a));
            SetLo16(ecx, closing);
            if ((int16_t)closing <= 0) continue;

            if ((GP4_FIELD(uint8_t, self, 0x8d) & 2u) != 0u) {
                if ((GP4_FIELD(uint8_t, other, 0x8d) & 2u) == 0u) continue;
            } else {
                if ((GP4_FIELD(uint8_t, other, 0x8d) & 2u) != 0u) continue;
            }

            if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, other, 0x9c) & 2u) == 0u) {
                SetLo16(eax, (uint16_t)(GP4_FIELD(uint16_t, other, 0xe2) - GP4_FIELD(uint16_t, self, 0xe2)));
            } else {
                gp4::Regs call = *r;
                call.eax = eax; call.ebx = ebx; call.ecx = ecx; call.edx = edx; call.edi = entry;
                gp4::call_regs(0x0040dc8a, call);
                eax = call.eax; ecx = call.ecx; edx = call.edx;
            }
            const uint16_t gap = (uint16_t)eax;
            if ((int16_t)gap < GP4_GLOBAL(int16_t, 0x006079e4)) continue;

            const int32_t lhs = (int32_t)(int16_t)gap * (int32_t)GP4_GLOBAL(int16_t, 0x00934df0);
            GP4_GLOBAL(uint16_t, 0x00934de0) = (uint16_t)lhs;
            GP4_GLOBAL(uint16_t, 0x00934de2) = (uint16_t)((uint32_t)lhs >> 16);
            const uint16_t cx = (uint16_t)ecx;
            const int32_t rhs = (int32_t)(int16_t)cx * (int32_t)GP4_GLOBAL(int16_t, 0x006079e6);
            SetLo16(edx, (uint16_t)((uint32_t)rhs >> 16));
            eax = (uint32_t)rhs;
            if (rhs >= GP4_GLOBAL(int32_t, 0x00934de0)) {
                hit = true;
                break;
            }
        }
    }

    uint8_t flagsByte = GP4_FIELD(uint8_t, self, 0xff);
    flagsByte = hit ? (uint8_t)(flagsByte | 0x10u) : (uint8_t)(flagsByte & 0xefu);
    GP4_FIELD(uint8_t, self, 0xff) = flagsByte;

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | LogicFlags8(flagsByte);
}
GP4_IMPL(0x004314be, FUN_004314be, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+list_0066fe3c+pair_0042e8e6+gap_0043530b+carff_004314be")
