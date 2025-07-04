const std = @import("std");

pub const Engine = struct {
    pub const name = "zig-engine";
    pub const author = "Konstantinos Despoinidis (KDesp73)";
    const VERSION_MAJOR = 0;
    const VERSION_MINOR = 3;
    const VERSION_PATCH = 0;

    pub fn version(major: *u32, minor: *u32, patch: *u32) void
    {
        major.* = VERSION_MAJOR;
        minor.* = VERSION_MINOR;
        patch.* = VERSION_PATCH;
    }

    pub fn welcome() !void {
        var stdout = std.io.getStdOut().writer();

        var major: u32 = 0;
        var minor: u32 = 0;
        var patch: u32 = 0;
        version(&major, &minor, &patch);
        _ = try stdout.print("{s} v{}.{}.{} by {s}\n", .{name, major, minor, patch, author});
    }

};
