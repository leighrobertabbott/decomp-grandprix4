#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Unpacks a block whose 0x20-byte header is at esi (the caller passes [0x008914e8]): header dwords
// +0/+4/+8/+0xc/+0x10/+0x1c go to 0x934dfc/0x868508/0x934de0/0x934de4/0x934de8/0x934dec, +0x14/+0x18
// become pointers into the buffer 0x7c8db4 (0x868514/0x868510). The [0x934de0] bytes after the header
// (int3 unless < 200000) are copied to 0x7c8db4; 0x866eec = end of the data + 4, 0x86850c and esi =
// 0x7c8db4 + header offsets, 0x8911f0 = (word [0x934dec]) + 0x10 (int3 unless that + 1 <= 0x80).
// Then the first (word [0x934dec]) dwords of the copied data are offsets of records: each becomes a
// pointer (+0x7c8db4); the record's first word must be >= 0x8003 (int3) and its nine dwords +4..+0x24
// (and 0x868508) are rebased by 0x7c8db4 - [0x934dfc]. Flags: the final `sub cx, 1`.

static inline uint32_t buffer_base() {
    return (uint32_t)reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c8db4));
}

// FUNCTION: GP4 0x00553dbd
void __cdecl FUN_00553dbd(gp4::Regs* r) {
    const uint32_t* header = (const uint32_t*)(uintptr_t)r->esi;
    GP4_GLOBAL(uint32_t, 0x00934dfc) = header[0];
    GP4_GLOBAL(uint32_t, 0x00868508) = header[1];
    GP4_GLOBAL(uint32_t, 0x00934de0) = header[2];
    GP4_GLOBAL(uint32_t, 0x00934de4) = header[3];
    GP4_GLOBAL(uint32_t, 0x00934de8) = header[4];
    uint32_t eax = header[5] + buffer_base();
    GP4_GLOBAL(uint32_t, 0x00868514) = eax;
    eax = header[6] + buffer_base();
    GP4_GLOBAL(uint32_t, 0x00868510) = eax;
    GP4_GLOBAL(uint32_t, 0x00934dec) = header[7];

    const uint8_t* src = (const uint8_t*)(uintptr_t)(r->esi + 0x20u);
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x00934de0);
    if (!((int32_t)length < 200000)) {
        __debugbreak();
    }
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c8db4);
    for (uint32_t left = length; left != 0u; --left) {
        *dst++ = *src++;
    }
    uint32_t edi = (uint32_t)reinterpret_cast<uintptr_t>(dst);
    GP4_GLOBAL(uint32_t, 0x00866eec) = (uint32_t)reinterpret_cast<uintptr_t>(src) + 4u;

    uint32_t ebx = GP4_GLOBAL(uint32_t, 0x00934de4) + buffer_base();
    GP4_GLOBAL(uint32_t, 0x0086850c) = ebx;
    const uint32_t esi = GP4_GLOBAL(uint32_t, 0x00934de8) + buffer_base();

    uint32_t ecx = (uint32_t)GP4_GLOBAL(uint16_t, 0x00934dec) + 0x10u;
    GP4_GLOBAL(uint32_t, 0x008911f0) = ecx;
    ecx += 1u;
    if (!(ecx <= 0x80u)) {
        __debugbreak();
    }

    uint16_t cx = GP4_GLOBAL(uint16_t, 0x00934dec);
    uint32_t arith = gp4::flags::sub16(cx, 1u);
    cx = (uint16_t)(cx - 1u);
    if ((int16_t)cx >= 0) {
        eax = buffer_base() - GP4_GLOBAL(uint32_t, 0x00934dfc);
        GP4_GLOBAL(uint32_t, 0x00868508) += eax;
        uint32_t* entry = (uint32_t*)GP4_ARRAY(uint8_t, 0x007c8db4);
        for (;;) {
            *entry += buffer_base();
            ebx = *entry;
            ++entry;
            uint32_t* record = (uint32_t*)(uintptr_t)ebx;
            if (!(*(const uint16_t*)record >= 0x8003u)) {
                __debugbreak();
            }
            record[1] += eax;
            record[2] += eax;
            record[3] += eax;
            record[4] += eax;
            record[5] += eax;
            record[6] += eax;
            record[7] += eax;
            record[8] += eax;
            record[9] += eax;
            arith = gp4::flags::sub16(cx, 1u);
            cx = (uint16_t)(cx - 1u);
            if ((int16_t)cx < 0) {
                break;
            }
        }
        edi = (uint32_t)reinterpret_cast<uintptr_t>(entry);
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = (ecx & 0xffff0000u) | cx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00553dbd, FUN_00553dbd, "regs(esi:ptr[0x1428]:layout=hdr_00553dbd) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of)")
