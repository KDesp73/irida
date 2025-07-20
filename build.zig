const std = @import("std");
const Janitor = @import("janitor.zig").Janitor;

pub fn build(b: *std.Build) void {
    var j = Janitor.init(b);
    j.exe("chess-engine");
    j.clib("castro", "castro/src/castro.h", "castro/src/", "castro/libcastro.a");
    j.install();

    j.step(.clean);
    j.step(.run);
}
