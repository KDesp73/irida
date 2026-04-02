# Irida

A UCI compatible chess engine written in C

<img src="https://github.com/user-attachments/assets/8e2208bf-b31c-4ad8-af9f-8252af5e178d" width=500>

## Get Started

> Run `make` to see what's available

### Build

```bash
make irida
```

### Install

```bash
make install
```

## Engine Design

### Move Generation

Move generation is handled by [castro](https://github.com/KDesp73/castro)
> TODO: expand on movegen

### Evaluation

Both hand-crafted and NNUE evaluation are available
> TODO: expand on evaluation

### Search

The search is based on the negamax algorithm with plenty of optimizations.

+ Iterative Deepening
+ Alpha-Beta Pruning
+ Quiescence
+ Move Ordering (MVV-VLA)
+ Transposition Table
+ Null Move Pruning
+ Late Move Reductions
+ Aspiration Windows
+ Principal Variation Search

### Syzygy

Syzygy is enabled by default as long as the tablebase can be located


## License

Currently closed-source

## Author

Konstantinos Despoindis ([KDesp73](https://github.com/KDesp73))

> This engine is a work-in-progress, developed with learning, experimentation, and future research in mind.
