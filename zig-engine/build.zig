const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "zigchess",
        .root_source_file = b.path("src/main.zig"),
        .target = b.graph.host,
        .optimize = optimize,
    });

    exe.addIncludePath(b.path("../movegen/include/"));
    exe.addObjectFile(b.path("../movegen/libcastro.a"));
    exe.linkLibC();

    b.installArtifact(exe);
}

