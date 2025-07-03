const std = @import("std");

const castro = @cImport({
    @cInclude("castro.h");
});

pub fn main() void {
    castro.InitMasks();
    castro.InitZobrist();

    var board: castro.Board = .{};

    castro.BoardInitFen(&board, castro.STARTING_FEN);

    castro.BoardPrint(&board, castro.SQUARE_NONE);

    const moves = castro.GenerateMoves(&board, castro.MOVE_LEGAL);
    std.debug.print("Legal moves count: {}\n", .{moves.count});
}
