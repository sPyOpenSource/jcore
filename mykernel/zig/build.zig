const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    var target = std.zig.CrossTarget{
        .os_tag = .freestanding,
        .cpu_arch = .x86_64,
        .abi = .none,
    };

    const mode = b.standardReleaseOptions();
    const kernel = b.addExecutable("mykernel.x86_64.elf", "src/main.zig");
    kernel.setLinkerScriptPath(.{ .path = "src/link.ld" });
    kernel.code_model = .kernel;
    kernel.setTarget(target);
    kernel.setBuildMode(mode);
    kernel.install();
    kernel.strip = true;
}
