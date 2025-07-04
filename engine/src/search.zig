const std = @import("std");
const eval = @import("eval.zig");
const log =  @import("log.zig");
const order = @import("moveordering.zig");


const c = @cImport({
    @cInclude("castro.h");
});

pub const MAX_PLY = 128;

pub const Searcher = struct {
    min_depth: usize = 1,
    max_millis: u64 = 0,
    ideal_time: u64 = 0,
    force_thinking: bool = false,
    iterative_deepening_depth: usize = 4,
    timer: std.time.Timer = undefined,

    soft_max_nodes: ?u64 = null,
    max_nodes: ?u64 = null,

    time_stop: bool = false,

    nodes: u64 = 0,
    ply: u32 = 0,
    seldepth: u32 = 0,
    stop: bool = false,
    is_searching: bool = false,

    nmp_min_ply: u32 = 0,

    hash_history: std.ArrayList(u64) = undefined,
    eval_history: [MAX_PLY]i32 = undefined,
    move_history: [MAX_PLY]c.Move = undefined,
    moved_piece_history: [MAX_PLY]c.Piece = undefined,

    best_move: c.Move = undefined,
    pv: [MAX_PLY][MAX_PLY]c.Move = undefined,
    pv_size: [MAX_PLY]usize = undefined,

    killer: [MAX_PLY][2]c.Move = undefined,
    history: [2][64][64]i32 = undefined,

    // counter_moves: [2][64][64]c.Move = undefined,
    // continuation: *[12][64][64][64]i32,

    pub fn init(alloc: std.mem.Allocator) !Searcher {
        var sh = Searcher{};
        sh.hash_history = std.ArrayList(u64).init(alloc);
        return sh;
    }

    pub fn deinit(self: *Searcher) void {
        self.hash_history.deinit();
    }

    fn checkStop(self: *Searcher) void {
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

    pub fn run(self: *Searcher, board: *c.Board) !void {
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

        // start at depth 1 up to min_depth (or further if you like)
        var depth: usize = 1;
        while (!self.stop and depth <= self.iterative_deepening_depth) : (depth += 1) {
            const score = self.searchDepth(board, depth, alpha_base, beta_base);

            var move_buf: [6:0]u8 = undefined;            // 5 chars + NUL
            c.MoveToString(self.best_move, &move_buf);    // write c‑string
            const move_slice = std.mem.sliceTo(move_buf[0..], 0);

            log.info("depth {} score cp {} nodes {} pv {s}",
                .{ depth, score, self.nodes, move_slice });

            self.checkStop();
        }

        self.is_searching = false;
    }

    fn searchDepth(
        self : *Searcher,
        board: *c.Board,
        depth: usize,
        alpha: i32,
        beta : i32,
    ) i32 {
        if (self.stop) return 0;

        self.nodes += 1;

        if (depth == 0) {
            return eval.eval(board);
        }

        var moves = c.GenerateLegalMoves(board);
        order.sortMoves(board, &moves);

        if (moves.count == 0) {
            // mate / stalemate
            const mate_val: i32 = 100_000;
            if (c.IsInCheck(board)) {
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

            // play
            if (!c.MakeMove(board, m)) continue;  // skip illegal (shouldn’t happen)

            self.ply += 1;
            const score = -self.searchDepth(board, depth - 1, -beta, -alpha_local);
            self.ply -= 1;

            c.UnmakeMove(board);

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
};
