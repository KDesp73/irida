const std = @import("std");
const log = @import("log.zig");

const c = @cImport({
    @cInclude("castro.h");
});

pub const UciOptionType = enum(u8) {
    check,   // true / false
    spin,    // integer spin
    combo,   // drop‑down value
    string,  // free text
};

pub const UciStatus = enum(u8) {
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

pub const UciOptionValue = union(enum) {
    check : bool,
    spin  : i32,
    combo : []const u8,
    string: []const u8,
};

pub const UciOptionParams = union(enum) {
    range  : SpinRange,
    combos : ComboList,
    none: void
};

pub const UciOption = struct {
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
    board              : c.Board = undefined,
    allocator          : std.mem.Allocator,

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

        std.mem.copyForwards(u8, self.start_position_fen[0..], c.STARTING_FEN[0..]);
        c.BoardInitFen(&self.board, &self.start_position_fen);
        try self.populateOptions();
        return self;
    }

    pub fn deinit(self: *Uci) void {
        self.uci_options.deinit();
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

        // TODO: welcome message
        
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
        c.BoardPrint(&self.board, c.SQUARE_NONE);
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
        _ = self;
        log.bestmove("e2e4", null);
    }

    fn position(self: *Uci, tokens: anytype) !void {
        var fen_builder = std.ArrayList(u8).init(self.allocator);
        defer fen_builder.deinit();

        if (tokens.next()) |token| {
            if (std.mem.eql(u8, token, "startpos")) {
                try fen_builder.appendSlice(c.STARTING_FEN);
            } else if (std.mem.eql(u8, token, "fen")) {
                while (tokens.next()) |t| {
                    if (std.mem.eql(u8, t, "moves")) break;

                    try fen_builder.append(' ');
                    try fen_builder.appendSlice(t);
                }
            }
        }

        const fen = try fen_builder.toOwnedSlice();
        defer self.allocator.free(fen);

        while(tokens.next()) |token| {
            if(token.len == 0) break;
            if(std.mem.eql(u8, token, "moves")) continue;

            var move_buf: [9:0]u8 = undefined;          // 8 bytes + sentinel
            const n = if(token.len < move_buf.len - 1) token.len else move_buf.len - 1;
            std.mem.copyForwards(u8, move_buf[0..n], token[0..n]);
            move_buf[n] = 0;                            // NULL‑terminate

            const move: c.Move = c.StringToMove(&move_buf);
            if (!c.MakeMove(&self.board, move)) {
                log.info("string Illegal move in position: {s}", .{ token });
                break;
            }
        }
    }

    fn stop(self: *Uci) !void {
        self.status = .stopped;
        log.info("Calculation stopped.", .{});
    }

    fn uci(self: *Uci) !void {
        self.uci_mode = true;

        log.id("name", "engine");
        log.id("author", "KDesp73");

        var it = self.uci_options.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.*.print();
        }

        log.uciok();
    }

    fn ucinewgame(self: *Uci) !void {
        self.deinit();
        self.* = try Uci.init(self.allocator);
        log.info("New game started.", .{});
    }
};

