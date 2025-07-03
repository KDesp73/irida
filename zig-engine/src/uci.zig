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
        var buf: []u8 = try self.allocator.alloc(u8, 256);
        defer self.allocator.free(buf);
        var rdr = std.io.getStdIn().reader();

        // TODO: welcome message

        while (self.status != .quit) {
            const n = try rdr.read(buf);
            if (n == 0) break;

            self.last_command = buf[0..n];
            self.handle(self.last_command) catch |err| switch (err) {
                error.BadCommand => {
                    log.info("Invalid command", .{});
                },
                error.OutOfMemory => {
                    log.info("Not enough memory", .{});
                }
            };
        }
    }

    fn handle(self: *Uci, input: []u8) !void {
        if(std.mem.startsWith(u8, input, "debug")) {
            try self.debug();
        } else if(std.mem.startsWith(u8, input, "D")) {
            self.display();
        } else if(std.mem.startsWith(u8, input, "ucinewgame")) {
            try self.ucinewgame();
        } else if(std.mem.startsWith(u8, input, "go")) {
            try self.go();
        } else if(std.mem.startsWith(u8, input, "isready")) {
            try self.isready();
        } else if(std.mem.startsWith(u8, input, "position")) {
            try self.position();
        } else if(std.mem.startsWith(u8, input, "quit")) {
            try self.quit();
        } else if(std.mem.startsWith(u8, input, "setoption")){
            try self.setoption();
        } else if(std.mem.startsWith(u8, input, "uci")){
            try self.uci();
        } else if(std.mem.startsWith(u8, input, "stop")){
            try self.stop();
        }
    } 

    fn debug(self: *Uci) !void {
        var it = std.mem.splitScalar(u8, self.last_command, ' ');

        const cmd = it.next() orelse return error.BadCommand;
        if (!std.mem.eql(u8, cmd, "debug"))
            return error.BadCommand;

        var arg = it.next() orelse {
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

    fn setoption(self: *Uci) !void {
        _ = self;
    }

    fn go(self: *Uci) !void {
        _ = self;
        log.bestmove("e2e4", null);
    }

    fn position(self: *Uci) !void {
        var fen_buf: [128:0]u8 = .{0} ** 128;
        var moves_slice: []const u8 = &[_]u8{};
        const line = self.last_command;

        if (std.mem.startsWith(u8, line, "position startpos")) {
            _ = std.mem.copyForwards(u8, &fen_buf, c.STARTING_FEN[0..]);

            if (std.mem.indexOf(u8, line, "moves")) |idx| {
                moves_slice = line[idx + "moves".len + 1 ..];
            }
        } else if (std.mem.startsWith(u8, line, "position fen ")) {
            const fen_start = line["position fen ".len ..];

            const moves_idx = std.mem.indexOf(u8, fen_start, " moves ");
            const fen_part = if (moves_idx != null and moves_idx.? != 0) fen_start[0 .. moves_idx.?] else fen_start;

            const copy_len = if(fen_part.len < fen_buf.len - 1) fen_part.len else fen_buf.len - 1;
            _ = std.mem.copyForwards(u8, fen_buf[0..copy_len], fen_part[0..copy_len]);

            if (moves_idx != null and moves_idx != 0) {
                moves_slice = fen_start[moves_idx.? + " moves ".len ..];
            }
        } else {
            log.info("string Invalid position command", .{});
            return;
        }

        _ = std.mem.copyForwards(u8, self.start_position_fen[0 .. fen_buf.len - 1],
            fen_buf[0 .. fen_buf.len - 1]);
        c.BoardInitFen(&self.board, &self.start_position_fen);

        if (moves_slice.len != 0) {
            var tokenizer = std.mem.splitScalar(u8, moves_slice, ' ');
            while (tokenizer.next()) |tok| {
                if (tok.len == 0) continue;

                var move_buf: [9:0]u8 = undefined;          // 8 bytes + sentinel
                const n = if(tok.len < move_buf.len - 1) tok.len else move_buf.len - 1;
                std.mem.copyForwards(u8, move_buf[0..n], tok[0..n]);
                move_buf[n] = 0;                            // NULL‑terminate

                const move: c.Move = c.StringToMove(&move_buf);
                if (!c.MakeMove(&self.board, move)) {
                    log.info("string Illegal move in position: {s}", .{ tok });
                    break;
                }
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
        // log.info("New game started.", .{});
    }
};

