const std = @import("std");
const NativeTargetInfo = std.zig.system.NativeTargetInfo;

pub fn build(b: *std.build.Builder) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const target_info = try NativeTargetInfo.detect(target);
    const name = switch (target_info.target.cpu.arch) {
        inline else => |arch| "mykernel." ++ @tagName(arch) ++ ".elf",
    };
    const kernel = b.addExecutable(.{
        .name = name,
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    kernel.setLinkerScriptPath(.{ .path = "src/link.ld" });
    kernel.code_model = switch (target_info.target.cpu.arch) {
        .x86_64 => .kernel,
        .riscv64 => .medium,
        .aarch64 => .large,
        else => |arch| {
            std.debug.print("unsupported arch: {}", .{arch});
            std.os.exit(1);
        },
    };
    kernel.strip = true;
    b.installArtifact(kernel);
}
