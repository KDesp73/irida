const std = @import("std");
const uci = @import("uci.zig");
const eval = @import("eval.zig");
const castro = @import("castro.zig");

pub fn main() !void {
    castro.init();
    eval.init_tables();

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    var Uci = try uci.Uci.init(gpa.allocator());
    defer Uci.deinit();

    try Uci.run();
}
