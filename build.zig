const std = @import("std");
const Janitor = @import("janitor.zig");

pub fn build(b: *std.Build) void {
    var j = Janitor.init(b);
    j.exe("chess-engine");
    j.clib("castro", "castro/src/castro.h", "castro/src/", "castro/libcastro.a");
    j.install();

    const cleanStep = j.step(.clean);
    _ = j.step(.run);
    _ = j.step(.help);

    const buildCastroStep = j.customStep("build-castro", "Build C movegen library", makeBuildCastroStep);
    j.b.default_step.dependOn(buildCastroStep);

    const cleanAllStep = j.customStep("cleanall", "Clean everything", makeCleanAllStep);
    cleanAllStep.dependOn(cleanStep);
}

fn makeCleanAllStep(step: *std.Build.Step, _: std.Build.Step.MakeOptions) anyerror!void {
    const allocator = step.owner.allocator;

    const rc = try Janitor.exec(allocator, &.{"make", "-C", "castro", "clean"});
    if(rc != 0) return error.CleanAllStepFailed;
}

fn makeBuildCastroStep(step: *std.Build.Step, _: std.Build.Step.MakeOptions) anyerror!void {
    const allocator = step.owner.allocator;

    const rc = try Janitor.exec(allocator, &.{"make", "-C", "castro", "all"});
    if(rc != 0) return error.BuildCastroStepFailed;
}
