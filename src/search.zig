const std    = @import("std");
const eval   = @import("eval.zig");
const log    = @import("log.zig");
const order  = @import("moveordering.zig");
const castro = @import("castro.zig");

const Searcher = @This();

pub const MAX_PLY = 128;


min_depth: usize = 1,
max_millis: u64 = 0,
iterative_deepening_depth: usize = 5,
timer: std.time.Timer = undefined,

time_stop: bool = false,

nodes: u64 = 0,
ply: u32 = 0,
seldepth: u32 = 0,
stop: bool = false,
is_searching: bool = false,

best_move: castro.lib.Move = undefined,

pub fn init(alloc: std.mem.Allocator) !Searcher {
    _ = alloc;
    const sh = Searcher{};
    return sh;
}

pub fn deinit(self: *Searcher) void {
    _ = self;
}

fn check_stop(self: *Searcher) void {
    // Stop if time limit reached or external stop flag set
    if (self.max_millis != 0) {
        const elapsed = self.timer.read();
        if (elapsed >= self.max_millis * 1000) {
            self.time_stop = true;
            self.stop = true;
        }
    }
    // TODO: other stopping conditions
}

pub fn run(self: *Searcher, board: *castro.lib.Board) !void {
    self.is_searching = true;
    self.timer = try std.time.Timer.start();

    // reset per‑search stats
    self.nodes       = 0;
    self.ply         = 0;
    self.seldepth    = 0;
    self.stop        = false;
    self.time_stop   = false;

    const alpha_base = -100_000_000;
    const beta_base  =  100_000_000;

    var depth: usize = 1;
    while (!self.stop and depth <= self.iterative_deepening_depth) : (depth += 1) {
        const score = self.search_depth(board, depth, alpha_base, beta_base);

        var move_buf: [6:0]u8 = undefined;
        castro.lib.MoveToString(self.best_move, &move_buf);
        const move_slice = std.mem.sliceTo(move_buf[0..], 0);

        log.info("depth {} score cp {} nodes {} pv {s}\r",
            .{ depth, score, self.nodes, move_slice });

        self.check_stop();
    }

    self.is_searching = false;
}

fn search_depth(
    self : *Searcher,
    board: *castro.lib.Board,
    depth: usize,
    alpha: i32,
    beta : i32,
) i32 {
    if (self.stop) return 0;

    self.nodes += 1;

    if (depth == 0) {
        return eval.eval(board);
    }

    var moves = castro.lib.GenerateLegalMoves(board);
    order.sortMoves(board, &moves);

    if (moves.count == 0) {
        // mate / stalemate
        const mate_val: i32 = 100_000;
        if (castro.lib.IsInCheck(board)) {
            // losing: negative score, shorter mate is worse
            return -mate_val + @as(i32, @intCast(self.ply));
        } else {
            return 0; // stalemate = draw
        }
    }

    var best_score: i32   = -100_000_000;
    var alpha_local: i32  = alpha;

    for (0..moves.count) |i| {
        const m = moves.list[i];

        if (!castro.lib.MakeMove(board, m)) continue;  // skip illegal (shouldn’t happen)

        self.ply += 1;
        const score = -self.search_depth(board, depth - 1, -beta, -alpha_local);
        self.ply -= 1;

        castro.lib.UnmakeMove(board);

        if (self.stop) return 0;

        if (score > best_score) {
            best_score = score;

            if (self.ply == 0) {
                self.best_move = m;
            }
        }

        if (best_score > alpha_local) {
            alpha_local = best_score;
        }

        if (alpha_local >= beta) break;
    }

    return best_score;
}
