CC = gcc
CFLAGS = -O3 -march=native -c -o $@
SRC = Toasty
LIBS = -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -lwinmm -static -std=c++17

all: perft chess

perft: $(SRC)/perft.c $(SRC)/board.c $(SRC)/move.c $(SRC)/bitboard.c $(SRC)/evaluate.c
	$(CC) -O3 -march=native -o perft.exe $^

chess: toasty.exe bitboard.exe board.exe move.exe evaluate.exe opening.exe search.exe hashmap.exe tinycthread.exe
	g++ -o $@ $^ $(LIBS)

toasty.exe: toasty.cpp
	g++ -c -o $@ $^ $(LIBS)

bitboard.exe: $(SRC)/bitboard.c $(SRC)/move.h
	$(CC) $(CFLAGS) $<

board.exe: $(SRC)/board.c $(SRC)/bitboard.h $(SRC)/move.h
	$(CC) $(CFLAGS) $<

evaluate.exe: $(SRC)/evaluate.c $(SRC)/board.h $(SRC)/bitboard.h $(SRC)/move.h
	$(CC) $(CFLAGS) $<

hashmap.exe: $(SRC)/hashmap.c
	$(CC) $(CFLAGS) $<

move.exe: $(SRC)/move.c $(SRC)/bitboard.h $(SRC)/board.h $(SRC)/evaluate.h
	$(CC) $(CFLAGS) $<

opening.exe: $(SRC)/opening.c $(SRC)/board.h $(SRC)/move.h
	$(CC) $(CFLAGS) $<

search.exe: $(SRC)/search.c $(SRC)/tinycthread.h $(SRC)/opening.h $(SRC)/board.h $(SRC)/evaluate.h $(SRC)/move.h $(SRC)/hashmap.h
	$(CC) $(CFLAGS) $<

tinycthread.exe: $(SRC)/tinycthread.c
	$(CC) $(CFLAGS) $<

clean:
	del *.exe