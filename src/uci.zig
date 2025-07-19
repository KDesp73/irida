const std = @import("std");
const log = @import("log.zig");
const engine = @import("engine.zig");
const eval = @import("eval.zig");
const search = @import("search.zig");
const cutils = @import("cutils.zig");
const castro = @import("castro.zig");

const UciOptionType = enum(u8) {
    check,   // true / false
    spin,    // integer spin
    combo,   // drop‑down value
    string,  // free text
};

const UciStatus = enum(u8) {
    waiting,
    start_thinking,
    thinking,
    stopped,
    ready,
    quit,
    pondering,
    setup_position,
    bestmove_sent,
};

const SpinRange   = struct { min: i32, max: i32 };
const ComboList   = [10][64]u8;

const UciOptionValue = union(enum) {
    check : bool,
    spin  : i32,
    combo : []const u8,
    string: []const u8,
};

const UciOptionParams = union(enum) {
    range  : SpinRange,
    combos : ComboList,
    none: void
};

const UciOption = struct {
    name          : []const u8,
    type          : UciOptionType,
    value         : UciOptionValue,
    params        : UciOptionParams,
    default_value : []const u8,

    pub fn print(self: UciOption) void {
        const w = std.io.getStdOut().writer();

        w.print("option name {s} type ", .{ self.name }) catch return;

        switch (self.type) {
            .check => {
                w.print("check", .{}) catch return;
                if (self.default_value.len != 0)
                    w.print(" default {s}", .{ self.default_value }) catch return;
                },

                .spin => {
                    w.print("spin", .{}) catch return;
                    if (self.default_value.len != 0)
                        w.print(" default {s}", .{ self.default_value }) catch return;
                    if (self.params == .range) {
                        const r = self.params.range;
                        w.print(" min {} max {}", .{ r.min, r.max }) catch return;
                    }
                },

                .combo => {
                    w.print("combo", .{}) catch return;
                    if (self.default_value.len != 0)
                        w.print(" default {s}", .{ self.default_value }) catch return;

                    if (self.params == .combos) {
                        const combos = self.params.combos;
                        for (&combos) |*cstr| {
                            if (cstr[0] == 0) break;
                            const s = std.mem.sliceTo(std.mem.asBytes(cstr)[0..], 0);
                            w.print(" var {s}", .{ s }) catch return;
                        }
                    }
                },

                .string => {
                    w.print("string", .{}) catch return;
                    if (self.default_value.len != 0)
                        w.print(" default {s}", .{ self.default_value }) catch return;
                    },
                }

        w.writeByte('\n') catch {};
    }
};

pub const Uci = struct {
    start_position_fen : [128]u8 = .{0} ** 128,
    uci_mode           : bool    = false,
    debug_mode         : bool    = false,
    depth_limit        : i32     = 0,
    time_left          : [2]i32  = .{ 0, 0 },
    increment          : [2]i32  = .{ 0, 0 },
    max_bookmoves      : i32     = 0,
    moves_to_go        : i32     = 0,
    ponder_mode        : bool    = false,
    infinite_mode      : bool    = false,
    stop_requested     : bool    = false,
    status             : UciStatus = .waiting,
    last_command       : [] u8,
    uci_options        : std.StringHashMap(UciOption),
    board              : castro.lib.Board = undefined,
    allocator          : std.mem.Allocator,

    searcher: search.Searcher = undefined,

    pub fn init(alloc: std.mem.Allocator) !Uci {
        var self = Uci{
            .allocator = alloc,
            .uci_mode     = true,
            .debug_mode   = true,
            .depth_limit  = 4,
            .max_bookmoves = 10,
            .uci_options  = std.StringHashMap(UciOption).init(alloc),
            .last_command = "",
        };

        self.searcher = try search.Searcher.init(alloc);

        std.mem.copyForwards(u8, self.start_position_fen[0..], castro.lib.STARTING_FEN[0..]);
        self.board = castro.board_init(null);
        try self.populateOptions();
        return self;
    }

    pub fn deinit(self: *Uci) void {
        self.uci_options.deinit();
        self.searcher.deinit();
    }

    fn populateOptions(self: *Uci) !void {
        try self.uci_options.put("Hash", UciOption {
            .name = "Hash",
            .type = .spin,
            .value = .{ .spin = 64 },
            .params = .{ .range = .{ .min = 1, .max = 2048 } },
            .default_value = "64",
        });

        try self.uci_options.put("Ponder", UciOption {
            .name = "Ponder",
            .type = .check,
            .value = .{ .check = false },
            .params = .none,
            .default_value = "false",
        });

        try self.uci_options.put("SkillLevel", UciOption {
            .name = "SkillLevel",
            .type = .spin,
            .value = .{ .spin = 20 },
            .params = .{ .range = .{ .min = 0, .max = 20 } },
            .default_value = "20",
        });

        try self.uci_options.put("SyzygyPath", UciOption {
            .name = "SyzygyPath",
            .type = .string,
            .value = .{ .string = "" },
            .params = .none,
            .default_value = ""
        });

        try self.uci_options.put("SyzygyProbeDepth", UciOption {
            .name = "SyzygyProbeDepth",
            .type = .spin,
            .value = .{ .spin = 1 },
            .params = .{ .range = .{ .min = 1, .max = 100 } },
            .default_value = "1"
        });

        try self.uci_options.put("Syzygy50MoveRule", UciOption {
            .name = "Syzygy50MoveRule",
            .type = .check,
            .value = .{ .check = true },
            .params = .none,
            .default_value = "true",
        });

        try self.uci_options.put("SyzygyProbeLimit", UciOption {
            .name = "SyzygyProbeLimit",
            .type = .spin,
            .value = .{ .spin = 7 },
            .params = .{ .range = .{ .min = 0, .max = 7 } },
            .default_value = "7"
        });
    }

    pub fn run(self: *Uci) !void {
        var stdin = std.io.getStdIn().reader();

        try engine.Engine.welcome();
        
        while (self.status != .quit) {
            const line = try stdin.readUntilDelimiterOrEofAlloc(self.allocator, '\n', 16384);

            if (line == null) {
                break;
            }

            const tline = std.mem.trim(u8, line.?, "\r");

            self.handle(tline) catch |err| switch (err) {
                error.NoCommand => {
                    log.info("No command found", .{});
                },
                error.UnknownCommand => {
                    log.info("Unknown command", .{});
                },
                error.BadCommand => {
                    log.info("Bad command", .{});
                },
                error.OutOfMemory => {
                    log.info("Not enough memory", .{});
                },
                error.TimerUnsupported => {
                    log.info("Unsupported timer", .{});
                }
            };
        }
    }

    fn handle(self: *Uci, input: []const u8) !void {
        var tokens = std.mem.splitScalar(u8, input, ' ');

        const token_opt = tokens.next();
        if (token_opt) |token| {
            if (std.mem.eql(u8, token, "debug")) {
                try self.debug(&tokens);
            } else if (std.mem.eql(u8, token, "d")) {
                self.display();
            } else if (std.mem.eql(u8, token, "ucinewgame")) {
                try self.ucinewgame();
            } else if (std.mem.eql(u8, token, "go")) {
                try self.go();
            } else if (std.mem.eql(u8, token, "isready")) {
                try self.isready();
            } else if (std.mem.eql(u8, token, "position")) {
                try self.position(&tokens);
            } else if (std.mem.eql(u8, token, "quit")) {
                try self.quit();
            } else if (std.mem.eql(u8, token, "setoption")) {
                try self.setoption(&tokens);
            } else if (std.mem.eql(u8, token, "uci")) {
                try self.uci();
            } else if (std.mem.eql(u8, token, "stop")) {
                try self.stop();
            } else {
                return error.UnknownCommand;
            }
        } else {
            return error.NoCommand;
        }
    }

    fn debug(self: *Uci, tokens: anytype) !void {
        var arg = tokens.next() orelse {
            log.info("debug is currently {s}\n",
                .{ if (self.debug_mode) "on" else "off" });
            return;
        };
        arg = std.mem.trim(u8, arg, "\n");

        if (std.mem.eql(u8, arg, "on")) {
            self.debug_mode = true;
        } else if (std.mem.eql(u8, arg, "off")) {
            self.debug_mode = false;
        } else {
            return error.BadCommand;
        }

        log.info("debug mode set to {s}\n", .{arg});
    }

    fn display(self: *Uci) void {
        castro.lib.BoardPrint(&self.board, castro.lib.SQUARE_NONE);
    }

    fn isready(self: *Uci) !void {
        _ = self;
        log.readyok();
    }

    fn quit(self: *Uci) !void {
        self.status = .quit;
    }

    fn setoption(self: *Uci, tokens: anytype) !void {
        _ = self;
        _ = tokens;
    }

    fn go(self: *Uci) !void {
        try self.searcher.run(&self.board);

        const move   = self.searcher.best_move;
        const buf    = try self.allocator.alloc(u8, 6); // 5 chars + NUL
        defer self.allocator.free(buf);

        castro.lib.MoveToString(move, @as([*c]u8, @ptrCast(buf.ptr)));

        const move_str = std.mem.sliceTo(buf, 0);

        log.bestmove(move_str, null);
    }

    fn position(self: *Uci, tokens: anytype) !void {
        var fen_builder = std.ArrayList(u8).init(self.allocator);
        defer fen_builder.deinit();

        if (tokens.next()) |token| {
            if (std.mem.eql(u8, token, "startpos")) {
                try fen_builder.appendSlice(castro.lib.STARTING_FEN);
            } else if (std.mem.eql(u8, token, "fen")) {
                while (tokens.next()) |t| {
                    if (std.mem.eql(u8, t, "moves")) break;

                    try fen_builder.append(' ');
                    try fen_builder.appendSlice(t);
                }
            }
        }

        const fen = try fen_builder.toOwnedSlice();
        const tfen = std.mem.trim(u8, fen, " ");
        defer self.allocator.free(fen);

        self.board = castro.board_init(tfen);

        while(tokens.next()) |token| {
            if(token.len == 0) break;
            if(std.mem.eql(u8, token, "moves")) continue;

            const move: castro.lib.Move = castro.parse_move(token);
            if (!castro.lib.MakeMove(&self.board, move)) {
                log.info("string Illegal move in position: {s}", .{ token });
                break;
            }
        }
    }

    fn stop(self: *Uci) !void {
        self.searcher.stop = true;
        log.info("Calculation stopped.", .{});
    }

    fn uci(self: *Uci) !void {
        self.uci_mode = true;

        log.id("name", engine.Engine.name);
        log.id("author", engine.Engine.author);

        var it = self.uci_options.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.*.print();
        }

        log.uciok();
    }

    fn ucinewgame(self: *Uci) !void {
        self.deinit();
        self.* = try Uci.init(self.allocator);
        // log.info("New game started.", .{});
    }
};

