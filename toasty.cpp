// Alex Eidt
// Toasty Chess Engine.

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

extern "C" {
	#include "Toasty/board.h"
	#include "Toasty/move.h"
	#include "Toasty/search.h"
	#include "Toasty/hashmap.h"
}

#define CAPTURE_AUDIO 0
#define END_AUDIO 1
#define MOVE_AUDIO 2
#define WARNING_AUDIO 3

#define BOARD_STATE "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

class Chess : public olc::PixelGameEngine {
public:
	Chess() {
		sAppName = "Toasty";
	}

private:
	int screenWidth, screenHeight;

	Board* chessboard;
	HashMap* table;

	std::unordered_map<byte, std::unordered_map<byte, olc::Decal*>> pieces;
	std::vector<int> audio;

	// Moves the player can make on every square of the board.
	std::vector<Move*> possible_moves[64];
	// Previously selected square.
	int selectedSource;
	int selectedDestination;

	// Moves the player can make.
	Move moves[MAX_MOVES];

	// Size of one square on the chess board.
	int unit;
	// Size of a chess piece sprite in pixels.
	float pieceSize;
	// Colors for the chess board.
	olc::Pixel light, dark, background, highlight;

	// Flags for Pawn Promotions.
	bool isPromotion;
	Piece promoted, promotedHover;

	bool gameOver;
	Piece winner; // WHITE, BLACK, or DRAW.

	// Used to track duplicate move destination spots to avoid redrawing circles.
	std::vector<int> destinations;

public:
	bool OnUserCreate() override {
		std::filesystem::path dir(std::filesystem::current_path().string());
		std::filesystem::path spriteDir("Pieces");
		std::filesystem::path audioDir("Audio");

		olc::SOUND::InitialiseAudio(44100, 1, 8, 512);

		std::vector<std::string> files;
		for (const auto& file : std::filesystem::directory_iterator(dir / audioDir)) {
			files.push_back(file.path().string());
		}

		std::sort(files.begin(), files.end());
		// Load Audio wav files from the "audio" directory into the audio vector.
		for (const auto& file : files) {
			int id = olc::SOUND::LoadAudioSample(file);
			if (id == -1) throw std::runtime_error("Failed to load audio sample: " + file);
			audio.push_back(id);
		}

		// Load Sprite png files from the "pieces" directory into the pieces vector.
		for (const auto& file : std::filesystem::directory_iterator(dir / spriteDir)) {
			std::string filepath = file.path().string();
			olc::Sprite* sprite = new olc::Sprite(filepath);
			if (sprite == nullptr) throw std::runtime_error("Failed to load piece: " + filepath);

			std::string piece = file.path().stem().string();
			Piece type;
			switch (piece[0]) {
				case 'p': type = PAWN; break;
				case 'n': type = KNIGHT; break;
				case 'b': type = BISHOP; break;
				case 'r': type = ROOK; break;
				case 'q': type = QUEEN; break;
				case 'k': type = KING; break;
				default: throw std::runtime_error("Invalid piece type: " + piece[0]);
			}

			Piece color;
			switch (piece[1]) {
				case 'b': color = BLACK; break;
				case 'w': color = WHITE; break;
				default: throw std::runtime_error("Invalid color: " + piece[1]);
			}

			pieces[color][type] = new olc::Decal(sprite);
		}

		srand(std::time(0));

		screenWidth = ScreenWidth();
		screenHeight = ScreenHeight();
		unit = std::min(screenWidth, screenHeight) / 10;
		pieceSize = pieces[WHITE][ROOK]->sprite->width;

		light = {235, 236, 208};
		dark = {119, 149, 86};
		background = {49, 46, 43};
		highlight = {246, 246, 105};

		isPromotion = false;
		promoted = EMPTY;
		promotedHover = EMPTY;

		gameOver = false;
		winner = 0;
		
		selectedSource = -1;
		selectedDestination = -1;

		chessboard = new Board();
		board_from_fen(chessboard, BOARD_STATE);
		table = hashmap_alloc(20);

		DrawBoard();
		GenerateMoves();

		// std::clock_t start;
		// uint64_t expected[] = {1, 20, 400, 8902, 197281, 4865609, 119060324};
		// for (int i = 0; i < sizeof(expected) / sizeof(uint64_t); i++) {
		// 	start = std::clock();
		// 	uint64_t actual = perft(i);
		// 	std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
		// 	printf("Depth: %d, Expected: %d, Actual: %d\n", i, expected[i], actual);
		// }

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		olc::vi2d mouse = {GetMouseX(), GetMouseY()};
		DrawPieces();

		if (gameOver) {
			DrawStringDecal({10, 10}, winner == DRAW ? "Draw" : "Checkmate", {255, 255, 255}, {5, 5});
			return true;
		}

		// Draw selected piece at mouse position.
		if (selectedSource != -1 && !isPromotion) {
			Piece piece = get_piece(chessboard, selectedSource);
			Piece color = get_color(chessboard, selectedSource);
			if (piece != EMPTY && color == chessboard->active_color && !possible_moves[selectedSource].empty()) {
				olc::vf2d scale = {float(unit / pieceSize), float(unit / pieceSize)};
				DrawDecal({float(mouse.x - unit / 2), float(mouse.y - unit / 2)}, pieces[color][piece], scale);
			}
		}

		if (isPromotion) {
			DrawPromotion();
			// Check if piece is a Queen, Bishop, Rook or Knight.
			if (promoted != EMPTY) {
				isPromotion = false;
				HandlePromotion();
				promoted = EMPTY;
				promotedHover = EMPTY;
			}
			return true;
		}
		
		// If the user clicks on a piece, show possible moves and allow user to make a move.
		if (GetMouse(0).bPressed) {
			int file = 7 - ((mouse.x - unit) / unit);
			int rank = 7 - ((mouse.y - unit) / unit);

			if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
				DrawBoard();
				DrawPieces();

				int index = rank * 8 + file;

				Move* selected = nullptr;
				if (selectedSource != -1) {
					for (const auto& move : possible_moves[selectedSource]) {
						if (move->to == index) {
							selected = move;
							break;
						}
					}
				}

				if (selected != nullptr) {
					selectedDestination = index;
					MakeMoveGUI(selected);
				} else {
					selectedSource = index;
					if (!possible_moves[selectedSource].empty()) {
						// Highlight the starting position.
						olc::vi2d rfs = Itov(selectedSource);
						FillRect({unit * rfs.x + unit, unit * rfs.y + unit}, {unit, unit}, highlight);
						ShowPossibleMoves();
					}
				}
			}
		}

		return true;
	}

	bool OnUserDestroy() {
		olc::SOUND::DestroyAudio();
		delete chessboard;
		hashmap_free(table);

		return true;
	}

	// Integer to vector. Flattened index to rank and file.
	olc::vi2d Itov(int index) {
		return {7 - (index % 8), 7 - (index / 8)};
	}

	// Show Possible moves from the selected source square.
	void ShowPossibleMoves() {
		SetPixelMode(olc::Pixel::ALPHA);
		destinations.clear();
		for (const auto& move : possible_moves[selectedSource]) {
			// Check for duplicate destination spots.
			if (std::find(destinations.begin(), destinations.end(), move->to) != destinations.end()) continue;
			destinations.push_back(move->to);

			olc::vi2d rf = Itov(move->to);
			olc::vi2d center = {rf.x * unit + unit * 3 / 2, rf.y * unit + unit * 3 / 2};
			if (IS_CAPTURE(move->flags) && !IS_EN_PASSANT(move->flags)) {
				FillCircle(center, unit / 2.1, {0, 0, 0, 100});
				FillCircle(center, unit / 2.75, (rf.x + rf.y) % 2 != 0 ? dark : light);
			} else {
				FillCircle(center, unit / 5, {0, 0, 0, 100});
			}
		}
		SetPixelMode(olc::Pixel::NORMAL);
	}

	// Make a move on the chess board.
	void MakeMove(Move* move) {
		// First, the player makes their move.
		make_move(chessboard, move);

		// Then the computer selects a move.
		Move* selected = new Move();
		if (select_move(chessboard, table, selected)) {
			make_move(chessboard, selected);
		} else {
			gameOver = true;
			winner = WHITE;
		}

		delete selected;

		// Then all possible moves are generated for the player.
		GenerateMoves();
	}

	// Make a move to the selected destination on the GUI.
	void MakeMoveGUI(Move* move) {
		// Highlight the source and destination squares of the most recently made move.
		olc::vi2d rfs = Itov(selectedSource), rfd = Itov(selectedDestination);
		FillRect({unit * rfs.x + unit, unit * rfs.y + unit}, {unit, unit}, highlight);
		FillRect({unit * rfd.x + unit, unit * rfd.y + unit}, {unit, unit}, highlight);

		// If the move was a pawn promotion, show the pawn promotion GUI to allow user to select the
		// piece they want to promote to (Queen, Bishop, Rook, Knight).
		if (IS_PROMOTION(move->flags)) {
			isPromotion = true;
		} else {
			if (IS_CAPTURE(move->flags) || IS_CASTLE(move->flags)) {
				olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
			} else {
				olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
			}
			MakeMove(move);
		}
	}

	// Once the user selects a piece to promote, make the corresponding move on the board.
	void HandlePromotion() {
		for (const auto& move : possible_moves[selectedSource]) {
			if (move->to == selectedDestination && IS_PROMOTION(move->flags) && PROMOTED_PIECE(move->flags) == promoted) {
				if (IS_CAPTURE(move->flags)) {
					olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
				} else {
					olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
				}

				MakeMove(move);
				DrawBoard();

				// Highlight the source and destination squares of the most recently made move.
				olc::vi2d rfs = Itov(selectedSource), rfsd = Itov(selectedDestination);
				FillRect({unit * rfs.x + unit, unit * rfs.y + unit}, {unit, unit}, highlight);
				FillRect({unit * rfsd.x + unit, unit * rfsd.y + unit}, {unit, unit}, highlight);
				break;
			}
		}
	}

	// Draw Screen for Pawn Promotions allowing user to selected promoted piece.
	void DrawPromotion() {
		int cx = screenWidth / 2;
		int cy = screenHeight / 2;
		int trim = unit / 5;
		int size = (4 * unit - 2 * trim) / 2;
		FillRect({cx - size - trim, cy - size - trim}, {(size + trim) * 2, (size + trim) * 2}, {0, 0, 0});
		FillRect({cx - size, cy - size}, {size * 2, size * 2}, {245, 245, 245});

		olc::vf2d scale = {float(size / pieceSize), float(size / pieceSize)};

		olc::vf2d top_left = {float(cx - size), float(cy - size)};
		olc::vf2d top_right = {float(cx), float(cy - size)};
		olc::vf2d bottom_left = {float(cx - size), float(cy)};
		olc::vf2d bottom_right = {float(cx), float(cy)};

		auto overlap = [](olc::vf2d p1, olc::vf2d p2, int size) {
			return p2.x <= p1.x && p1.x <= p2.x + size && p2.y <= p1.y && p1.y <= p2.y + size;
		};
		
		olc::vf2d mouse = {float(GetMouseX()), float(GetMouseY())};

		bool tl = overlap(mouse, top_left, size);
		bool tr = overlap(mouse, top_right, size);
		bool bl = overlap(mouse, bottom_left, size);
		bool br = overlap(mouse, bottom_right, size);

		olc::Pixel normal = {255, 255, 255, 255};
		olc::Pixel tint = {255, 255, 255, 100};

		DrawDecal(top_left, pieces[chessboard->active_color][QUEEN], scale, tl ? tint : normal);
		DrawDecal(top_right, pieces[chessboard->active_color][ROOK], scale, tr ? tint : normal);
		DrawDecal(bottom_left, pieces[chessboard->active_color][BISHOP], scale, bl ? tint : normal);
		DrawDecal(bottom_right, pieces[chessboard->active_color][KNIGHT], scale, br ? tint : normal);

		if (GetMouse(0).bPressed) {
			if (tl) promoted = QUEEN;
			else if (tr) promoted = ROOK;
			else if (bl) promoted = BISHOP;
			else if (br) promoted = KNIGHT;
			else promoted = EMPTY;
		}

		if (tl) promotedHover = QUEEN;
		else if (tr) promotedHover = ROOK;
		else if (bl) promotedHover = BISHOP;
		else if (br) promotedHover = KNIGHT;
		else promotedHover = EMPTY;
	}

	// Draws the chess board.
	void DrawBoard() {
		Clear(background);
		int trim = unit / 4;
		FillRect({unit - trim, unit - trim}, {8 * unit + 2 * trim, 8 * unit + 2 * trim}, {0, 0, 0});

		for (int file = 0; file < 8; file++) {
			for (int rank = 0; rank < 8; rank++) {
				olc::vf2d pos = {float(rank * unit + unit), float(file * unit + unit)};
				FillRect(pos, {unit, unit}, (file + rank) % 2 != 0 ? dark : light);
			}
		}
	}

	// Draw the pieces on the board.
	void DrawPieces() {
		olc::vf2d scale = {float(unit / pieceSize), float(unit / pieceSize)};
		olc::Pixel tint = olc::Pixel(255, 255, 255, 255);
		bool isValid = !possible_moves[selectedSource].empty();

		for (int file = 0; file < 8; file++) {
			for (int rank = 0; rank < 8; rank++) {
				// If the promotion screen is up, then do not draw pieces on ranks 3-6 and files c-f
				// since this area is covered by the promotion selection GUI.
				if (isPromotion && file >= 2 && file <= 5 && rank >= 2 && rank <= 5) continue;
				int index = file * 8 + rank;
				Piece piece = get_piece(chessboard, index);
				Piece color = get_color(chessboard, index);

				tint.a = 255;
				if ((isValid && index == selectedSource) || (index == selectedDestination && isPromotion)) tint.a = 50;
				olc::vi2d rf = Itov(index);

				olc::Decal* sprite;
				if (promotedHover != EMPTY && index == selectedDestination) {
					sprite = pieces[get_color(chessboard, selectedSource)][promotedHover];
				} else if (piece != EMPTY) {
					sprite = pieces[color][piece];
				} else {
					continue;
				}
				DrawDecal({float(rf.x * unit + unit), float(rf.y * unit + unit)}, sprite, scale, tint);
			}
		}
	}

	// Generate all possible moves for the current player.
	void GenerateMoves() {
		for (int i = 0; i < 64; i++) {
			possible_moves[i].clear();
		}

		int legal = gen_moves(chessboard, moves);

		// If a player cannot make any more moves, then they have lost.
		if (legal == 0) {
			// if player is in check then declare winner, otherwise it's a draw.
			gameOver = true;
			winner = OPPOSITE(chessboard->active_color);
			return;
		}

		for (int i = 0; i < legal; i++) {
			Move* move = &moves[i];
			possible_moves[move->from].push_back(move);
		}
	}

	uint64_t perft(int depth) {
		Board* board = new Board();
		board_from_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

		return helper(board, depth);
	}

	uint64_t helper(Board* board, int depth) {
		if (depth == 0) return 1ULL;

		Move moves[MAX_MOVES];
		int n_moves = gen_moves(board, moves);

		Board copy = *board;
		uint64_t nodes = 0;
		for (int i = 0; i < n_moves; i++) {
			make_move(board, &moves[i]);
			nodes += helper(board, depth - 1);
			*board = copy; // Undo move.
		}

		return nodes;
	}
};

int main() {
	Chess demo;
	if (demo.Construct(800, 800, 1, 1))
		demo.Start();

	return 0;
}