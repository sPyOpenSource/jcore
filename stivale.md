Critisism of Stivale2
=====================

After I've read through the [Stivale2](https://github.com/stivale/stivale/blob/master/STIVALE2.md) boot protocol specification, I
become 100% convinced that I must NOT use it. Not in the loader and not in a kernel, never ever. Here are some of my reasons (but
there's more):

- first, it is said to be a simple protocol. That's a lie. Just take a look at the spec, it is NOT simple, not by far.

- only considers ELF kernels, nothing else is supported, by design. WTF? Why does it talk later about anchors then?

- the creators demonstrated that they are unable to comprehend the difference between ELF's Linking View and Execution View (see
  [ELF spec page 46](https://www.sco.com/developers/devspecs/gabi41.pdf)), and messed up stivale2 because of it right from the start.

- the stivale2 header must be placed in a section, however section headers are optional and probably non-existent in executable
  files; they are only required in relocatable objects and only used by the linker, nothing else. Linkers (or strip) allowed to
  remove sections, keeping only the program headers with segments (and indeed they tend to do that).

- section headers (if they even exists at all) therefore reside at the end of the ELF file, meaning you must load the file entirely
  into memory (at the right address, which you obviously cannot know until you parse the stivale2 tags!!!) and you must parse ELF
  structures throughfully just to determine if the file is stivale compliant or not (this is an issue, because kernels can be
  several megabytes in size). There's a good reason why Multiboot2 mandates that its tags must appear in the first 32 Kbytes in the
  file, and stivale creators failed to understand that.

- stivale2 anchor likewise can appear anywhere in the file, so yet again, you must load the file entirely into memory just to
  determine if it's stivale compatible or not. Auch. (Actually worse, because now there's no header to guide you, you literally
  have to scan through the entire file.)

- stivale2 anchor does not have a file offset field, nor a file relative entry point offset, so it is totally unusable. And anchor
  has an absolute memory address for the stivale2 header inside the ELF... (same mistake as with Multiboot2's address tag, why not
  just use the file offsets like everybody else does?) The spec goes like this: "Anchored kernels are otherwise functionally
  equivalent to ELF counterparts." That's a joke, right? ELF has segments (with file relative offsets) and relocation records! Where
  are those basic functionalities in stivale2 anchors?

- when anchor is used, there supposed to be further tags, however those might not exist, which makes this protocol extremely fragile
  and prone to failure. And even in ELF files, tags are totally inconsistent: what if a kernel has "no linear framebuffer, use CGA
  text mode" (wait, what? CGA????) tag, but also has a "framebuffer" tag with the requirested resolution for example? What should a
  loader do then? How on earth is a RaspberryPi loader supposed to provide CGA text mode? What should the SMP tag's flag contain on
  an ARM machine? What if the flags are zero on x86? Etc. etc. etc. The stivale creators know nothing about portability and data
  consistency, that's painfully obvious. (And CGA was obsoleted in the mid - '90s, many many years before Multiboot2!)

- the stivale2 anchor and structures are full of hard coded memory addresses (as unit64_t numbers, so no ELF relocation records for
  them!!!), meaning you cannot relocate a stivale kernel easily. If UEFI happens to use that part of the memory already, then good
  luck booting that kernel on that machine! Stivale2 supports KASLR, that's another joke, right??? It can't even cope with everyday
  relocations!

- the tag structures are poorly designed, they use huge, memory-wasting identifiers, unaligned members and ain't binary-safe. Unless
  you explicitly tell your C compiler (in a compiler-specific way) to keep the structs packed, then they will likely to rearrange
  these fields (again, in a compiler- and platform-specific way). Should they use different compilers, there's no guarantee that a
  loader and a kernel will use the same binary structure even though they have included the very same stivale2.h header! You are
  totally at the mercy of your compiler (and its version). This is more than epic failure, this is just lame.

- the specification mandates that only the first 4G of RAM must be mapped, meaning this won't work on 64-bit machines where the
  MMIO or the framebuffer is above the 4G mark (like on my real hardware for example). Another epic fail, no comment. In contrast,
  BOOTBOOT maps the framebuffer at a kernel defined virtual address, whatever its physical address might be.

- allowing 5-level page tables is futile, and requires unnecessary extra work from the loaders. The boot time writable and executable
  identity mapping is unusable for any sane kernel, they will throw that away as soon as possible, so there's really no reason to
  overcomplicate it. It also wastes lots of RAM without any reason (as you cannot rely on the UEFI provided page tables, the loader
  must create a brand new set of page tables).

- the spec is inconsistent about the stack for 32-bit and 64-bit kernels. This is plain wrong. (Yes I understand that despite what
  the stivale2 spec claims, on 32-bits the ELF doesn't use SysV ABI, rather CDECL which happens to need a stack, but come on!)

- stivale2 expects that the loader is kept resident in memory, which wastes RAM and makes unnecessary restrictions on kernels. This
  one alone is enough to avoid stivale. Unlike Multiboot2, stivale2 mandates that its structures and code must be kept untouched,
  always available at FIXED addresses (!) in every address space! (Well, this isn't told in the spec per se, but it is obvious
  from the existence of absolute pointers and that AP cores are actively polling those stivale2 structures!)

- and guess what, you don't know which part of the memory must be kept! The memory map only reports boot loader reclaimable, but
  what happens if a kernel indeed reclaims and reuses those memory areas? Will it destroy its own mapping perhaps? Some areas are
  used by the APs as well! The memory map records are not even guaranteed to be distinct, even on a single core system you might
  just end up overwriting loader memory because of overlaping entries... A crash is guaranteed, and yet here we are again, stivale2
  is fragile and prone to failure protocol. (Multiboot2 reports these as free memory, so you know you must be careful, there's no
  false feeling of safety.)

- a stivale2 loader must provide not just the boot information (as with Multiboot2), but a whole lot of code as well, with arbitrary
  interfaces (like a totally unuseful 7-bit VT console for example; man, I haven't used any non-8-bit serial terminal, for what,
  four decades now? And it cannot print numbers, just strings? No varargs?) Implemeneting these interfaces are a lot of extra
  work without any benefit, simply does not worth the effort. Period. Any sane kernel developer would just implement their own
  (UTF-8 aware, vararg based and capable to display numbers) kprintf and that's it (and loosers would just copy'n'paste something
  like [this one](https://gitlab.com/bztsrc/simpleboot/-/blob/main/example/kernel.c#L168) into their kernels and go on with their
  lives). Adding function callbacks and 7-bit VT functionality is totally unnecessary overcomplication of the boot protocol. Yet
  another epic failure.

- but there's more, keeping an executable part of the loader and making it accessible from the kernel in supervisor mode (!) is an
  INSANELY HUGE security risk that no sane kernel developer would ever want to take. Perfect way to **inject malicious code** right
  into the **most privileged part** of any stivale-compatible kernel! Nice! (And just for the records, counteracts any KASLR as
  the mailicious code gets the kernel's data segment in argument as well as the text segment address on the stack.)

- in order to keep those interfaces working, a lot of restrictions are made to the kernel's design (fixed GDT, specific mapping,
  etc.) without a real reason. I simply cannot imagine why would any kernel developer want that. It just makes absolutely no sense.

No, thanks! I'd rather [tweak the Multiboot2 protocol a bit to support 64-bit kernels](https://gitlab.com/bztsrc/easyboot) instead.

Verdict: it is strongly recommended to any hobby OS kernel or boot loader developers to avoid stivale / stivale2. If you don't
believe me, see for yourself!
