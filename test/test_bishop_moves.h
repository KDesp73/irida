#ifndef test_bishop_moves_h
#define test_bishop_moves_h

#define TEST_test_bishop_moves\
	test_bishop_moves("rnb1kb1r/ppp1p1pp/5n2/3pPp2/7q/2N5/PPPP1PPP/R1BQKBNR w KQkq - 0 1", "c1", NULL), \
	test_bishop_moves("rnb1kb1r/ppp1p1pp/5n2/3pPp2/7q/2N5/PPPP1PPP/R1BQKBNR w KQkq - 0 1", "f1", "e2", "d3", "c4", "b5", "a6", NULL), \
	test_bishop_moves("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b Kkq - 0 1", "a6", "b5", "c4", "d3", "e2", "b7", "c8", NULL), \
	test_bishop_moves("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b Kkq - 0 1", "g7", "f8", "h6", NULL), \
	test_bishop_moves("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w Kkq - 0 1", "d2", "c1", "e3", "f4", "g5", "h6", NULL), \
	test_bishop_moves("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w Kkq - 0 1", "e2", "d1", "f1", "d3", "c4", "b5", "a6", NULL), \
	test_bishop_moves("r2q1rk1/2p1bppp/p1n1bn2/1p2p3/4P3/2P2N2/PPBN1PPP/R1BQR1K1 w - - 1 12", "c1", NULL), \
	test_bishop_moves("r2q1rk1/2p1bppp/p1n1bn2/1p2p3/4P3/2P2N2/PPBN1PPP/R1BQR1K1 w - - 1 12", "c2", "b1", "b3", "a4", "d3", NULL ), \
	test_bishop_moves("r2q1rk1/2p1bppp/p1n1bn2/1p2p3/4P3/2P2N2/PPBN1PPP/R1BQR1K1 b - - 1 12", "e7", "d6", "c5", "b4", "a3", NULL), \
	test_bishop_moves("r2q1rk1/2p1bppp/p1n1bn2/1p2p3/4P3/2P2N2/PPBN1PPP/R1BQR1K1 b - - 1 12", "e6", "d5", "c4", "b3", "a2", "d7", "c8", "f5", "g4", "h3", NULL), \
	test_bishop_moves("r1bqrbk1/2p2pp1/p1np1n1p/1p2p3/4P3/PBNP1N1P/1PPB1PP1/R2Q1RK1 w - - 3 12", "b3", "a2", "c4", "d5", "e6", "f7", "a4", NULL), \
	test_bishop_moves("r1bqrbk1/2p2pp1/p1np1n1p/1p2p3/4P3/PBNP1N1P/1PPB1PP1/R2Q1RK1 w - - 3 12", "d2", "c1", "e3", "f4", "g5", "h6", "e1", NULL)

#endif
