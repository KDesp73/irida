const std = @import("std");

// Import the C header
const castro = @cImport({
    @cInclude("castro.h");
});

pub fn main() void {
    var board: castro.Board = undefined;
    std.debug.print("Starting fen: {s}\n", .{castro.STARTING_FEN});
    castro.BoardInitFen(&board, castro.STARTING_FEN);

    castro.BoardPrint(&board, 64);

    const moves = castro.GenerateMoves(&board, castro.MOVE_LEGAL);

    std.debug.print("Legal moves count: {}\n", .{moves.count});
}

