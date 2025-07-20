const std = @import("std");
const Uci = @import("uci.zig").Uci;
const eval = @import("eval.zig");
const castro = @import("castro.zig");

pub fn main() !void {
    castro.init();
    eval.init();

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    var uci = try Uci.init(gpa.allocator());
    defer uci.deinit();

    try uci.run();
}
