# Dynamic storage

## Synopsis

Simple storage with dynamic memory (re-)allocation.

Goals:

1. Stable indexes (ids);
2. O(1) push back;
3. O(1) delete by id;
4. O(1) get by id;
5. Cache frendly iteration;
6. Single file.

### Implementation

- Data lies directly in storage (e.g., std::vector);
- Two auxiliary arrays provide forward and reverse indices, enabling stable ID generation and constant-time lookup;
- Deletion is O(1) via swap‑and‑pop.

## Licence

Licensed under the MIT license.

#### P.S.

I would be very grateful for finding bugs and expressing constructive criticism.