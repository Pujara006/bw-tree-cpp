# Bw-Tree C++

A from-scratch, incrementally-concurrent index in C++17, working toward a simplified
lock-free **Bw-Tree** in the spirit of the Microsoft Research paper
*"The Bw-Tree: A B-tree for New Hardware Platforms"* (Levandoski, Lomet, Sengupta).

The project is built in phases: rather than starting at the lock-free design, it starts
with a correct single-threaded B+ Tree and tightens the concurrency protocol one commit
at a time, with a test suite guarding every step.

| Phase | Goal | Status |
| --- | --- | --- |
| 1 | Single-threaded B+ Tree (insert, search, range scan, delete, validation) | Complete |
| 2 | Lock-based concurrency (coarse tree latch → fine-grained lock coupling) | In progress |
| 3 | Replace in-place updates with delta chains | Not started |
| 4 | Atomic CAS installs for lock-free updates | Not started |

## Build and test

Requires CMake ≥ 3.16 and a C++17 compiler.

```bash
./scripts/test.sh
```

That script does a clean build and runs the whole suite. To build only:

```bash
./scripts/build.sh
```

Or drive CMake directly, e.g. with AddressSanitizer enabled:

```bash
cmake -S . -B build -DENABLE_ASAN=ON && cmake --build build && ./build/bplus_tree_tests
```

CMake produces two targets: the `bplus_tree` library and the `bplus_tree_tests`
executable. `src/main.cpp` is a scratch demo that prints tree structure after a series
of inserts; it has no CMake target, so compile it by hand if you want it:

```bash
c++ -std=c++17 -Iinclude src/main.cpp src/bplus_tree.cpp -o my_program
```

## API

```cpp
#include "bplus_tree.hpp"

BPlusTree tree(4);              // order 4 => at most 3 keys per node; order must be >= 3

tree.insert(42, 420);           // duplicate keys are rejected (logged, no overwrite)

int value = 0;
if (tree.search(42, value)) { /* value == 420 */ }

auto pairs = tree.rangeSearch(10, 100);   // vector<pair<int,int>>, inclusive, sorted

tree.deleteKey(42);             // returns false if the key was absent

tree.validateTree();            // full structural invariant check
tree.printTree();               // level-by-level dump
tree.printLeaves();             // walk the leaf linked list
```

Keys and values are both `int`. The tree stores values only in leaves, which are
threaded into a singly linked list via `next` so range scans walk sideways instead of
re-descending.

## Design notes

**Node layout.** One `Node` struct serves both roles, tagged by `isLeaf`. Internal nodes
hold separator `keys` and `children`; leaves hold `keys`, `values`, and a `next` pointer.
Children are `shared_ptr`, so nodes dropped by a merge stay alive as long as some thread
still references them. Each node carries its own `std::shared_mutex`, and the tree carries
one more (`treeLock`) that guards the root pointer itself.

**Splits and merges.** Inserts split bottom-up: an overfull leaf pushes a separator into
its parent, which may split in turn and ultimately grow a new root. Deletes repair
underflow by borrowing from a sibling when one has a key to spare, and merging otherwise;
when the root ends up with a single child, `shrinkRoot` drops a level.

**Concurrency, as it currently stands:**

| Operation | Protocol |
| --- | --- |
| `search` | Shared **lock coupling** down the tree: latch the child, release the parent, return the leaf with its shared latch still held. Never touches `treeLock`. |
| `insert` | Exclusive lock coupling with a *safe-node* rule — the moment a child is found with room to spare, every ancestor latch is dropped, since nothing above it can split. `treeLock` is held only while the root might still be replaced. |
| `deleteKey` | Still coarse-grained: one exclusive `treeLock` for the whole operation. |
| `rangeSearch`, `printTree`, `printLeaves`, `validateTree` | Shared `treeLock` over the whole tree. |

Note the asymmetry this leaves mid-migration: because the fine-grained `insert` and
`search` paths deliberately avoid `treeLock`, delete's exclusive tree latch does not in
fact exclude them, and delete takes no node latches of its own. Delete has yet to be
converted to lock coupling — that is the next step of Phase 2, not a settled design.

## Tests

`bplus_tree_tests` is a plain assertion-based harness (no external framework); each case
prints `[PASS]` and aborts on failure. 56 cases across five suites:

| Suite | Cases | Covers |
| --- | --- | --- |
| [tests/test_insert_search.cpp](tests/test_insert_search.cpp) | 8 | Insert paths, splits, lookups, duplicates |
| [tests/test_range_search.cpp](tests/test_range_search.cpp) | 7 | Inclusive bounds, empty and partial ranges, leaf-chain walks |
| [tests/test_validation.cpp](tests/test_validation.cpp) | 3 | Structural invariants: key order, uniform leaf depth, leaf chain |
| [tests/test_delete.cpp](tests/test_delete.cpp) | 22 | Underflow, sibling borrow, leaf and internal merges, root shrink |
| [tests/test_concurrency.cpp](tests/test_concurrency.cpp) | 16 | Parallel search/insert/delete, split fallback, mixed workload, random stress |

## Layout

```
include/bplus_tree.hpp     public API and internal node/traversal types
src/bplus_tree.cpp         the implementation
src/main.cpp               scratch demo program (not a CMake target)
tests/                     assertion-based test suites
scripts/build.sh           clean CMake configure + build
scripts/test.sh            build, then run the test binary
```

## Known limitations

- `int` keys and values only; no templates, no variable-length payloads.
- Inserting an existing key logs to stdout and leaves the tree unchanged — there is no
  update-in-place or upsert.
- Delete is not yet lock-coupled (see the note above).
- No iterators, no persistence, no memory reclamation beyond `shared_ptr`.

## License

Apache License 2.0 — see [LICENSE](LICENSE).
