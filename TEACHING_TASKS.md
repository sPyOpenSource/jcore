# Teaching Tasks: x86-64 Long Mode Boot

## Phase 1: Understanding the Modes
- [ ] **Task 1**: Recap - What is real mode and why do we leave it?
- [ ] **Task 2**: Protected mode basics - What does it give us?
- [ ] **Task 3**: Long mode overview - Why 64-bit?

## Phase 2: Prerequisites
- [ ] **Task 4**: Memory layout and why we need page tables
- [ ] **Task 5**: The role of the GDT in long mode

## Phase 3: Implementation
- [x] **Task 6**: Walk through `loader.s` - the combined boot file
- [x] **Task 7**: Page tables deep dive - PML4, PDPT, PD structure
- [x] **Task 8**: Enabling PAE - What is CR4 bit 5? (IN PROGRESS)
- [x] **Task 9**: The EFER MSR and Long Mode Enable (LME) bit
- [x] **Task 10**: Enabling paging - CR0 bit 31
- [x] **Task 11**: Setting up the 64-bit GDT
- [x] **Task 12**: The far jump (`ljmp`) to 64-bit mode
- [x] **Task 13**: The 64-bit stub - setting up stack and segments
- [x] **Task 14**: Calling into C/C++ from 64-bit assembly

## Phase 4: Verification & Debugging
- [x] **Task 15**: How to verify you are in long mode
- [x] **Task 16**: Common triple fault causes and fixes
- [x] **Task 17**: QEMU debugging tips for boot code
- [x] **Task 18**: Reviewing the existing `boot32.s` and `boot64.s`
- [x] **Task 19**: Comparing old split approach vs combined `loader.s`
- [ ] **Task 20**: Exercises and challenges for the student

---
*Progress: 19/20 tasks completed*
