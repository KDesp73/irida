# movegen

Independent Move Generation Library

## Example

```c
#include <movegen/board.h>
#include <movegen/moves.h>

int main() {
    Board board = {0};
    BoardInitFen(&board, NULL); // Starting Position

    Moves moves = GenerateMoves(&board, MOVE_LEGAL);
    Move move = moves.list[0];
    MakeMove(&board, move);

    BoardPrintMove(&board, move);

    return 0;
}
```

## License

[MIT](./LICENSE)
