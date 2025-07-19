const std = @import("std");

pub fn build(b: *std.Build) void {
    const target   = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    
    const castro_bindings = b.addTranslateC(.{
        .target = target,
        .optimize = optimize,
        .use_clang = true,
        .link_libc = true,
        .root_source_file = b.path("./castro/src/castro.h"),
    });
    
    const exe = b.addExecutable(.{
        .name             = "engine",
        .root_source_file = b.path("src/main.zig"),
        .target           = target,
        .optimize         = optimize,
    });
    
    exe.root_module.addImport("castro", castro_bindings.createModule());
    exe.addIncludePath(b.path("./castro/src/"));
    exe.addObjectFile(b.path("./castro/libcastro.a"));
    exe.linkLibC();
    
    b.installArtifact(exe);
}
