#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct ScrollValue_00539798 {   // the item's value block (+0xe of the item)
    uint32_t Position;          // +0, written
    uint32_t Total;             // +4
    uint32_t Page;              // +8
};
#pragma pack(pop)

// 'mul b; cmp divisor, edx; jbe; div divisor': (a * b) / divisor when the quotient fits, else
// clampValue with edx = 0.
static inline uint32_t mulDivClamp(uint32_t a, uint32_t b, uint32_t divisor, uint32_t clampValue) {
    const uint64_t product = static_cast<uint64_t>(a) * b;
    if (divisor > static_cast<uint32_t>(product >> 32)) {
        uint32_t rem;
        return gp4::div64(product, divisor, &rem);
    }
    return clampValue;
}

// Menu-item handler for a vertical scroll bar (esi = item: i16 rectangle top +6 / bottom +0xa, value
// block pointer +0xe, flags +0x1e), mode in edx. Mode 1 does nothing, 5/6 clear [0x0082fd88];
// mode 4 updates only when +0x1e bit 2 is set. Mode 2 (press) computes the drag offset
// [0x0082fda0] from the pointer y [0x0082fd98]: 0 when the total is 0; fd98 - thumbTop - 1 when the
// pointer lies on the thumb (top + 1 + pos * span / total <= y <= top + 1 + (pos + page) * span /
// total), else half the page scaled to the span. Modes 2/3/4 then clear the repeat byte (0x0053daa6)
// and set pos = clamp((max(y - offset - top - 1, 0) * total / span) + page, .., total) - page,
// clamped to >= 0. edx returns 0; the flags are those of the last compare/sub on the path taken.
// FUNCTION: GP4 0x00539798
void __cdecl FUN_00539798(gp4::Regs* r) {
    uint8_t* const item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t mode = r->edx;
    uint32_t flags = 0u;

    bool update = false;
    if (mode == 1u) {
        flags = gp4::flags::sub32(mode, 1u);
    } else if (mode == 2u) {
        ScrollValue_00539798* const value = GP4_FIELD(ScrollValue_00539798*, item, 0xe);
        GP4_GLOBAL(uint32_t, 0x0082fda0) = 0u;
        if (value->Total != 0u) {
            const uint32_t top = static_cast<uint32_t>(GP4_FIELD(uint16_t, item, 6)) + 1u;
            const uint32_t span = static_cast<uint32_t>(GP4_FIELD(uint16_t, item, 0xa)) - top;
            bool onThumb = false;
            const uint32_t thumbEnd =
                mulDivClamp(value->Position + value->Page, span, value->Total, value->Total) + top;
            if (static_cast<int32_t>(thumbEnd) >= GP4_GLOBAL(int32_t, 0x0082fd98)) {
                const uint32_t thumbStart = mulDivClamp(value->Position, span, value->Total, value->Total) + top;
                if (static_cast<int32_t>(thumbStart) <= GP4_GLOBAL(int32_t, 0x0082fd98)) {
                    GP4_GLOBAL(uint32_t, 0x0082fda0) = GP4_GLOBAL(uint32_t, 0x0082fd98) - thumbStart - 1u;
                    onThumb = true;
                }
            }
            if (!onThumb) {
                GP4_GLOBAL(uint32_t, 0x0082fda0) = mulDivClamp(value->Page >> 1, span, value->Total, value->Total);
            }
        }
        update = true;
    } else if (mode == 3u) {
        update = true;
    } else if (mode == 4u) {
        const uint32_t bit = GP4_FIELD(uint8_t, item, 0x1e) & 4u;
        flags = gp4::flags::logic8(bit);
        update = bit != 0u;
    } else if (mode == 5u) {
        flags = gp4::flags::sub32(mode, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        flags = gp4::flags::sub32(mode, 6u);
        if (mode == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    if (update) {
        gp4::cdecl_<0x0053daa6, void>();
        const uint32_t top = static_cast<uint32_t>(GP4_FIELD(uint16_t, item, 6)) + 1u;
        const uint32_t span = static_cast<uint32_t>(GP4_FIELD(uint16_t, item, 0xa)) - top;
        const uint32_t pointer = GP4_GLOBAL(uint32_t, 0x0082fd98) - GP4_GLOBAL(uint32_t, 0x0082fda0);
        uint32_t offset = pointer - top;
        if (static_cast<int32_t>(pointer) < static_cast<int32_t>(top)) {
            offset = 0u;
        }
        ScrollValue_00539798* const value = GP4_FIELD(ScrollValue_00539798*, item, 0xe);
        uint32_t end = mulDivClamp(value->Total, offset, span, span) + value->Page;
        if (static_cast<int32_t>(end) > static_cast<int32_t>(value->Total)) {
            end = value->Total;
        }
        flags = gp4::flags::sub32(end, value->Page);
        uint32_t position = end - value->Page;
        if (static_cast<int32_t>(end) < static_cast<int32_t>(value->Page)) {
            position = 0u;
        }
        value->Position = position;
        r->eax = position;
        r->ebx = top;
        r->ecx = offset;
        r->ebp = span;
        r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value));
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539798, FUN_00539798, "regs(edx:u32{1,2,2,2,3,3,4,4,5,6,0,7}, esi:ptr[0x20]:layout=bar_00539798, eax:u32, ebx:u32, ecx:u32, ebp:u32, edi:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=thumb_00539798")
