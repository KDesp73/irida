const std = @import("std");

/// Allocate a C string from a Zig slice.
///
/// ‑ `allocator` : the allocator that will own the new buffer
/// ‑ `src`       : the Zig slice you want to hand to C
///
/// Returns `[:0]u8`  (a slice whose last byte is the sentinel 0).
///
/// Example:
/// ```zig
/// const cstr = try toCString(gpa.allocator(), "hello world");
/// defer gpa.allocator().free(cstr);
/// c.some_c_function(cstr.ptr);
/// ```
pub fn toCString(
    allocator: std.mem.Allocator,
    src: []const u8,
) ![:0]u8 {
    // +1 for the terminating NUL
    var out = try allocator.alloc(u8, src.len + 1);

    // copy data
    std.mem.copy(u8, out[0..src.len], src);

    // append sentinel
    out[src.len] = 0;

    // return as *sentinel* slice ([:0]u8)
    return out[0 .. src.len + 1 :0];
}

