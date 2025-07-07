const std = @import("std");
const uci = @import("uci.zig");
const eval = @import("eval.zig");

const castro = @cImport({
    @cInclude("castro.h");
});

pub fn main() !void {
    castro.InitMasks();
    castro.InitZobrist();
    eval.init_tables();

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    var Uci = try uci.Uci.init(gpa.allocator());
    defer Uci.deinit();

    try Uci.run();
}
