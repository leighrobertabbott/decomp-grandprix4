#include <gp4/gp4.h>

// FUNCTION: GP4 0x00427a53
// Lateral-grip look-ahead for the car at esi on the segment record at edi: the base term
// 0x603d48 = (+-(word [edi+2] * [0x622481] >> 14) * [0x603854]) >> 2 is always stored. A speed count
// is then chosen: no car link (+0x1bc != 0xffff) uses the segment's flag 0x20 and word +0x184 << 6;
// a free car takes either 0x428481 plus |+0x288 * [0x603854] >> 18| and |[0x62480c]|, or (flag bit +0xce 4,
// +0x163 0x80) the look-ahead scan 0x4284e1 with count [0x624848]. A zero count copies the limit
// [0x6247a8] to [0x6247b8]; otherwise 0x42790f's term is added to 0x603d48 and 0x427880 derives
// [0x6247b8]. The remaining registers are whatever the callees left.
void __cdecl FUN_00427a53(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;

    car[0x230] &= 0xdfu;

    const int64_t gripProduct = static_cast<int64_t>(GP4_FIELD(int16_t, seg, 2)) * GP4_GLOBAL(int32_t, 0x00622481);
    uint32_t grip = static_cast<uint32_t>(static_cast<uint64_t>(gripProduct) >> 14);
    if ((car[0x71] & 4u) != 0u) {
        grip = 0u - grip;
    }
    const int64_t baseProduct = static_cast<int64_t>(static_cast<int32_t>(grip)) * GP4_GLOBAL(int32_t, 0x00603854);
    st.eax = static_cast<uint32_t>(static_cast<uint64_t>(baseProduct) >> 2);
    st.edx = static_cast<uint32_t>(static_cast<uint64_t>(baseProduct) >> 32);
    GP4_GLOBAL(uint32_t, 0x00603d48) = st.eax;

    if (GP4_FIELD(uint16_t, car, 0x1bc) != 0xffffu) {
        if ((seg[0x12] & 0x20u) == 0u) {
            st.ecx = 0u;
        } else {
            st.ecx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x184)) << 6);
        }
    } else {
        bool scan = (car[0xce] & 4u) != 0u;
        if (!scan) {
            const int64_t speed = static_cast<int64_t>(GP4_FIELD(int32_t, car, 0x288)) * GP4_GLOBAL(int32_t, 0x00603854);
            st.eax = static_cast<uint32_t>(static_cast<uint64_t>(speed) >> 18);
            st.edx = static_cast<uint32_t>(static_cast<uint64_t>(speed) >> 32);
            st.ecx = st.eax;
            scan = (car[0x163] & 0x80u) != 0u;
        }
        if (!scan) {
            gp4::call_regs(0x00428481, st);
            const int64_t speed = static_cast<int64_t>(GP4_FIELD(int32_t, car, 0x288)) * GP4_GLOBAL(int32_t, 0x00603854);
            st.eax = static_cast<uint32_t>(static_cast<uint64_t>(speed) >> 18);
            st.edx = static_cast<uint32_t>(static_cast<uint64_t>(speed) >> 32);
            st.ecx = st.eax;
            if (static_cast<int32_t>(st.ecx) < 0) {
                st.ecx = 0u - st.ecx;
            }
            st.eax = GP4_GLOBAL(uint32_t, 0x0062480c);
            if (static_cast<int32_t>(st.eax) < 0) {
                st.eax = 0u - st.eax;
            }
        } else {
            st.eax = GP4_GLOBAL(uint32_t, 0x00624848);
            gp4::call_regs(0x004284e1, st);
            st.ecx = st.eax;
            GP4_GLOBAL(uint32_t, 0x0062fbf0) = st.eax;
            const int64_t speed = static_cast<int64_t>(GP4_FIELD(int32_t, car, 0x288)) * GP4_GLOBAL(int32_t, 0x00603854);
            st.eax = static_cast<uint32_t>(static_cast<uint64_t>(speed) >> 24);
            st.edx = st.eax;
            if (static_cast<int32_t>(st.edx) < 0) {
                st.edx = 0u - st.edx;
            }
            GP4_GLOBAL(uint32_t, 0x0062fbf4) = st.edx;
        }
    }

    uint32_t flags;
    if (st.ecx == 0u) {
        st.eax = GP4_GLOBAL(uint32_t, 0x006247a8);
        GP4_GLOBAL(uint32_t, 0x006247b8) = st.eax;
        flags = 0x44u;
    } else {
        if (static_cast<int32_t>(st.ecx) < 0) {
            st.ecx = 0u - st.ecx;
        }
        gp4::call_regs(0x0042790f, st);
        GP4_GLOBAL(uint32_t, 0x00603d48) += st.eax;
        gp4::call_regs(0x00427880, st);
        flags = st.eflags & 0x8d5u;
    }

    r->eax = st.eax;
    r->ebx = st.ebx;
    r->ecx = st.ecx;
    r->edx = st.edx;
    r->esi = st.esi;
    r->edi = st.edi;
    r->ebp = st.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00427a53, FUN_00427a53, "regs(edi:rec[0x8943a0,0x160,0..25], esi:ptr[0x2a0]:layout=rec_0042882e+car_0043c677+car_004154d5+rec_00408170) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+walk_00427f3c+state_00427d4b+state_00427880+state_004154d5")
