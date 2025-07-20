const std = @import("std");
const Janitor = @import("janitor.zig").Janitor;

pub fn build(b: *std.Build) void {
    var j = Janitor.init(b);
    j.exe("chess-engine");
    j.install();
    j.clib("castro", "castro/src/castro.h", "castro/src/", "castro/libcastro.a");

    const cleanStep = j.step(.clean);
    _ = j.step(.run);

    const buildCastroStep = j.customStep("build-castro", "Build C movegen library", makeBuildCastroStep);
    j.b.getInstallStep().dependOn(buildCastroStep);

    const cleanAllStep = j.customStep("cleanall", "Clean everything", makeCleanAllStep);
    cleanAllStep.dependOn(cleanStep);
    
}

fn makeCleanAllStep(step: *std.Build.Step, _: std.Build.Step.MakeOptions) anyerror!void {
    const allocator = step.owner.allocator;

    var process = std.process.Child.init(&[_][]const u8{
        "make", "-C", "castro", "clean"
    }, allocator);

    process.stderr_behavior = .Inherit;
    process.stdout_behavior = .Inherit;

    try process.spawn();
    const result = try process.wait();
    if (result.Exited != 0) return error.AutocompleteGenerationFailed;
}

fn makeBuildCastroStep(step: *std.Build.Step, _: std.Build.Step.MakeOptions) anyerror!void {
    const allocator = step.owner.allocator;

    var process = std.process.Child.init(&[_][]const u8{
        "make", "-C", "castro", "all"
    }, allocator);

    process.stderr_behavior = .Inherit;
    process.stdout_behavior = .Inherit;

    try process.spawn();
    const result = try process.wait();
    if (result.Exited != 0) return error.AutocompleteGenerationFailed;
}
