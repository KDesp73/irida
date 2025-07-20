// A zig wrapper for the castro.h library (v0.2.0)
// Useful mainly for string dependent functions
pub const lib = @import("castro");
const std = @import("std");

/// Converts a string like "e2e4" into a `lib.Move` struct.
pub fn parse_move(str: []const u8) lib.Move {
    var move_buf: [9]u8 = undefined; // 8 bytes + NULL terminator
    const n = @min(str.len, move_buf.len - 1);
    std.mem.copyForwards(u8, move_buf[0..n], str[0..n]);
    move_buf[n] = 0; // NULL-terminate

    return lib.StringToMove(@as([*c]const u8, @ptrCast(&move_buf)));
}

/// Converts a `lib.Move` to a heap-allocated Zig string (caller must free it).
pub fn move_to_string(allocator: std.mem.Allocator, move: lib.Move) ![]u8 {
    const buf = try allocator.alloc(u8, 6); // 5 chars + NULL terminator
    lib.MoveToString(move, @as([*c]u8, @ptrCast(buf.ptr)));

    // Get the null-terminated length
    const len = buf.len;
    return allocator.dupe(u8, buf[0..len]);
}

pub fn board_init(fen: ?[]const u8) lib.Board {
    var board: lib.Board = undefined;
    if(fen == null) {
        lib.BoardInitFen(@as([*c]lib.Board, @ptrCast(&board)), lib.STARTING_FEN);
        return board;
    }

    var c_fen: [256]u8 = undefined;
    std.mem.copyForwards(u8, c_fen[0..fen.?.len], fen.?);
    c_fen[fen.?.len] = 0; // null terminator

    lib.BoardInitFen(@as([*c]lib.Board, @ptrCast(&board)), &c_fen);
    return board;
}

pub fn init() void {
    lib.InitMasks();
    lib.InitZobrist();
}
