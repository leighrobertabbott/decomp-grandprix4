# Emulator input layouts

A spec argument `ptr[0x200]:layout=<name>` fills the buffer from
`<name>.json` instead of random bytes, so the original function sees plausible
data (finite floats, valid indices, real nested pointers) and its cases stay valid.

```json
{
  "size": 512,
  "fields": [
    {"off": 0,    "type": "f64", "lo": -3.15, "hi": 3.15},
    {"off": 8,    "type": "f32", "lo": 0,     "hi": 90},
    {"off": 16,   "type": "i32", "lo": 0,     "hi": 21},
    {"off": 20,   "type": "ptr", "size": 64,  "fill": "f32"}
  ]
}
```

Types: `i8 u8 i16 u16 i32 u32 f32 f64 ptr`. Unlisted bytes are zero. The lead
writes these alongside `include/gp4/types/*.h` when workers ask for a layout.

For fixed-address state, `; globals=layout=<name>` uses a layout with a
`globals` array. Each entry specifies `addr`, `type`, and optional `lo`/`hi`;
`count` and `stride` repeat a field across records. Addresses and strides may
be integers or hexadecimal strings. Only declared fields are varied, so a
byte discriminator can be tested without corrupting neighbouring pointers.
`search_0066d82c.json` records the fields read by `0x00409c13`.

Repeated `u8` fields may use `"pattern": "bit_scan", "bit": 128` to mix
independent bytes with all-clear and exactly-one-set-bit cases. Other bits
still vary. This exercises both early exits and an exhausted scan without
changing the function's memory model. The pattern cannot also use `lo`/`hi`.

More field options (all validated by `gp4re/input_layouts.py`):

- `"points_into": {"base": "0x8943a0", "stride": "0x160", "lo": 4, "hi": 26}` on
  a single `u32` global: the value is `base + k*stride` for a random k in
  lo..hi, i.e. a record-aligned pointer into a fixed array (end pointers,
  cursors). `records_008943a0.json` uses it.
- `"alias": "<addr of an earlier field>"`: in about half the cases the field
  holds the same bytes as that earlier field, otherwise its own value
  (equality selectors such as `cmp esi,[0x679d2c]` after `mov esi,[0x679d30]`).
- `"prefix": [0, 1, 5, 22]` on a repeated (`count` > 1) integer global field: draw n
  from the list, give the first n entries nonzero values (from `values` without 0, or
  random) and zero the rest - a table filled up to n slots (all slots used = `count`).
  Add `"alias_always": true` when the game keeps the two equal as an invariant
  (e.g. two counters an int3 assertion compares): then they are equal in every case.
- `"values": [0, "0x41c", "0xffff"]` on integer fields: pick from the list
  (offset tables with terminators, nonzero divisors with both signs as two's
  complement). Cannot be combined with `lo`/`hi`.
- On `ptr` fields: `"bias": "0x15e"` stores `buffer + bias` (the code reads at
  negative offsets), `"nullable": true` makes the pointer null in about a
  quarter of the cases.

Every layout carries an `_evidence` string: which instructions read the
fields and why the chosen domain is valid (no faults, both branch outcomes).

Pointer-buffer layouts (`size` + `fields`) accept the same repetition and
choice options: `"count"`/`"stride"` repeat a field across records inside the
buffer (`off + k*stride`, offsets may be hex strings), `"values"` picks from a
list, and `ptr` fields take `"nullable"` and `"bias"`. A field outside the
buffer is an error. `group_00431e58.json` uses all of these.

`"alias_reg": "esi"` on a single `u32` global: in about half the cases the
global holds the value of that register argument (equality selectors such as
`cmp esi,[0x66d820]`). Pair it with a spec register of type
`rec[base,stride,lo..hi]` (record-aligned address into a fixed array) when the
register must point into a game array whose fields the same layout varies.
A register that must stay a fixed number of records away from another one
(a `[edi,ebp)` range walk) uses the spec type `rel[reg,stride,lo..hi]`:
`ebp:rel[edi,0x160,1..12]` is edi + k*0x160 with k drawn from 1..12; `reg`
must be an earlier input of the same spec.

A spec may compose global layouts: `; globals=layout=state_0066d8c0+bits_x`
applies each in turn (later fields win where they overlap), so a function that
reads two families of state reuses both layouts instead of copying fields.

`"sign_of": "<addr of an earlier dword field>"` makes a dword the sign
extension of that field (0 or 0xffffffff): the high half of a 64-bit pair
`lo, hi = lo >> 31` as produced by `cdq`.

A pointer-buffer layout may set `"fill": "bytes"` at the top level: unlisted
bytes are then random instead of zero, and only the listed fields (valid
nested pointers, bounded indices, value lists) are pinned. Use it for big
records such as a car (0x41c bytes) where most bytes are flags/counters that
any pattern exercises but a few pointers must be valid.

Pointer-argument layouts compose like global ones: `ptr[n]:layout=a+b` builds
the buffer from `a` (its size and `fill` background), then overlays the fields
of `b` (and further names) on top.

Buffer-layout fields may also use `"points_into": {"base", "stride", "lo", "hi"}`
(a u32 holding a record address inside a fixed game array, optionally
`"nullable": true`), e.g. a car's segment pointer +0x1c. A global `"type": "ptr"`
field takes `"size"` and `"fill"`, and the fill may itself be `"layout=<name>"`
(a global pointer to a structured buffer such as a stream).

A buffer-layout field `{"off": 0, "type": "self", "bias": 0}` stores the address
of the buffer itself (+ `bias`; `count`/`stride` repeat it). A node whose link
dword points to itself makes a pointer chain of any depth valid (`mov esi,[esi]`
N times lands on the same node); a later field or `+` layout at the same offset
overrides it. Only allocated buffers (ptr fills, pointer arguments) can hold self
fields, not fixed-address globals. A chain that must terminate needs distinct nodes.


## File handles

The emulator's fake file system (gp4re/stubs.py) treats a handle value in 0x3000..0x3ffc (multiple of 4) that the run did not open itself as a file a caller opened earlier: it gets seeded size and contents on first use. Use such values (e.g. `"values": ["0x3000", "0x3004", "0xffffffff"]`) for handle globals that callers open before the function runs; any other unknown handle behaves as an invalid handle.


## Pointer pools

A repeated `ptr` field (count >= 2) may set `"pool": k` (1..64): k buffers are allocated per case and every entry points at one of them, chosen at random. Use it for large pointer tables (e.g. the 3136-entry text table in script_all_00835846) where one page-guarded buffer per entry would overflow the 16 MB input arena.
