const std    = @import("std");
const c      = @cImport({ @cInclude("castro.h"); });

const MoveOrdering = @This();

pub const PAWN_VALUE   = 100;
pub const KNIGHT_VALUE = 320;
pub const BISHOP_VALUE = 330;
pub const ROOK_VALUE   = 500;
pub const QUEEN_VALUE  = 900;
pub const KING_VALUE   = 0;      // usually 0 because the king is priceless

pub const PIECE_VALUES = [_]i32{
    PAWN_VALUE,
    KNIGHT_VALUE,
    BISHOP_VALUE,
    ROOK_VALUE,
    QUEEN_VALUE,
    KING_VALUE,
};

pub var MVV_LVA_SCORES: [12][12]i32 = undefined;

pub inline fn initMVVLVA() void {
    // 12 = 6 piece types × 2 colours
    for (0..12) |attacker| {
        for (0..12) |victim| {
            MVV_LVA_SCORES[attacker][victim] =
                PIECE_VALUES[victim % 6] * 10 -
                PIECE_VALUES[attacker % 6];
        }
    }
}

pub const PIECES: []const u8 = "pnbrqkPNBRQK";

inline fn pieceIndex(ch: u8) ?usize {
    return std.mem.indexOfScalar(u8, PIECES, ch);
}

inline fn gridChar(board: *const c.Board, sq: c.Square) u8 {
    const rank: usize = sq / 8;
    const file: usize = sq % 8;
    return @as(u8, @intCast(board.*.grid[rank][file]));
}

fn scoreMove(board: *c.Board, move: c.Move) i32 {
    const from      = c.GetFrom(move);
    const to        = c.GetTo(move);
    const promotion = c.GetPromotion(move);

    const attacker_ch = gridChar(board, from);
    const victim_ch   = gridChar(board, to);

    var score: i32 = 0;

    if (victim_ch != c.EMPTY_SQUARE) {
        const att_idx = pieceIndex(attacker_ch) orelse 0;
        const vic_idx = pieceIndex(victim_ch)   orelse 0;
        score += 100_000 + MVV_LVA_SCORES[att_idx][vic_idx];
    }

    var m = move;

    if (c.IsPromotion(board, &m)) {
        // prefer Queen > Rook > Bishop > Knight
        score += 80_000 + @as(i32, promotion) * 1_000;
    }

    if (c.IsCapture(board, move)) {
        score += 50_000; // generic capture bonus
    }

    if (c.IsInCheckAfterMove(board, move)) {
        score += 3_000;
    }

    return score;
}

const ScoredMove = struct {
    move : c.Move,
    score: i32,
};

pub fn sortMoves(board: *c.Board, moves: *c.Moves) void {
    var scored: [c.MOVES_CAPACITY]ScoredMove = undefined;

    for (0..moves.count) |i| {
        const m = moves.list[i];
        scored[i] = .{
            .move  = m,
            .score = scoreMove(board, m),
        };
    }

    const slice = scored[0 .. moves.count];

    std.sort.insertion(ScoredMove, slice, {}, struct {
        pub fn less(_: void, a: ScoredMove, b: ScoredMove) bool {
            return a.score > b.score;
        }
    }.less);

    for (0..moves.count) |i| {
        moves.list[i] = slice[i].move;
    }
}
