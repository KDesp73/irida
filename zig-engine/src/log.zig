const std = @import("std");

/// Write a raw line to stdout, swallowing I/O errors (GUI will simply not see
/// the line in that unlikely case).
inline fn _println(msg: []const u8) void {
    const w = std.io.getStdOut().writer();
    w.print("{s}\n", .{msg}) catch {};
}

/// Send an `info` line (printf‐style).
pub fn info(comptime fmt: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    _ = std.fmt.bufPrint(&buf, "info " ++ fmt, args) catch return;
    _println(&buf);
}

/// Send `id name ...`  or  `id author ...`
pub fn id(comptime what: []const u8, comptime value: []const u8) void {
    const line = std.fmt.comptimePrint("id {s} {s}", .{ what, value });
    _println(line);
}

/// Tell GUI that UCI handshake is finished.
pub inline fn uciok() void  { _println("uciok"); }

/// Tell GUI that `isready` has been answered.
pub inline fn readyok() void { _println("readyok"); }

/// Send the final best move (and optional ponder move).
pub fn bestmove(best: []const u8, ponder: ?[]const u8) void {
    const w = std.io.getStdOut().writer();

    if (ponder) |p| {
        w.print("bestmove {s} ponder {s}", .{ best, p }) catch return;
    } else {
        w.print("bestmove {s}", .{ best }) catch return;
    }
}
