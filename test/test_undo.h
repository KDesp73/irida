#ifndef test_undo_h
#define test_undo_h

#define TEST_test_undo\
	test_undo("rnbqkbnr/pppp1ppp/8/4p2Q/4P3/8/PPPP1PPP/RNB1KBNR b KQkq - 0 1", MoveEncodeNames("f7", "f6", PROMOTION_NONE, FLAG_NORMAL)), \
	test_undo("rnbqkbnr/ppp2p1p/6p1/2Ppp2Q/4P3/8/PP1P1PPP/RNB1KBNR b KQkq d6 0 1", MoveEncodeNames("g6", "h5", PROMOTION_NONE, FLAG_NORMAL)), \
	test_undo("rnbqkbnr/ppp2p1p/6p1/2Ppp2Q/4P3/8/PP1P1PPP/RNB1KBNR w KQkq d6 0 1", MoveEncodeNames("c5", "d6", PROMOTION_NONE, FLAG_ENPASSANT)), \
	test_undo("r1bqk2r/ppp2ppp/2n2n2/3Pp3/1b6/1B3N2/PPPP1PPP/RNBQK2R w KQkq - 0 1", MoveEncodeNames("e1", "g1", PROMOTION_NONE, FLAG_CASTLING)), \
	test_undo("r1bqk2r/ppp2ppp/2n2n2/3Pp3/1b6/1B3N2/PPPP1PPP/RNBQK2R b KQkq - 0 1", MoveEncodeNames("e8", "g8", PROMOTION_NONE, FLAG_CASTLING)), \
	test_undo("r3kbnr/pppqpppp/2n5/3p1b2/8/2NPB3/PPPQPPPP/R3KBNR w KQkq - 0 1", MoveEncodeNames("e1", "c1", PROMOTION_NONE, FLAG_CASTLING)), \
	test_undo("r3kbnr/pppqpppp/2n5/3p1b2/8/2NPB3/PPPQPPPP/R3KBNR b KQkq - 0 1", MoveEncodeNames("e8", "c8", PROMOTION_NONE, FLAG_CASTLING))

#endif
