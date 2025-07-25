const std = @import("std");

pub const Engine = struct {
    pub const name = "chess-engine";
    pub const author = "Konstantinos Despoinidis (KDesp73)";
    const version = "0.3.0";

    pub fn welcome() !void {
        var stdout = std.io.getStdOut().writer();

        _ = try stdout.print("{s} v{s} by {s}\n", .{name, version, author});
    }

};
