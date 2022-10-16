// Alex Eidt
// Pumpking Chess Engine.

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

extern "C" {
	#include "Pumpking/board.h"
	#include "Pumpking/move.h"
}

#define CAPTURE_AUDIO 0
#define END_AUDIO 1
#define MOVE_AUDIO 2
#define WARNING_AUDIO 3

class Chess : public olc::PixelGameEngine {
public:
	Chess() {
		sAppName = "Pumpking";
	}

private:
	int screenWidth, screenHeight;

	Board* chessboard;

	std::map<byte, std::map<byte, olc::Decal*>> pieces;
	std::vector<int> audio;

	// Moves the player can make as indices on the board.
	std::vector<Move*> possible[64];
	// Previously selected square.
	int selected;
	int selectedDestination;

	// Moves the player can make.
	Move moves[256];

	// Size of one square on the chess board.
	int unit;
	// Size of a chess piece sprite in pixels.
	float pieceSize;
	// Colors for the chess board.
	olc::Pixel light, dark, background, highlight;

	bool isPromotion;
	Piece promoted;

public:
	bool OnUserCreate() override {
		std::filesystem::path dir (std::filesystem::current_path().string());
		std::filesystem::path spriteDir ("Pieces");
		std::filesystem::path audioDir ("Audio");

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
		promoted = 0;
		
		selected = -1;
		selectedDestination = -1;

		chessboard = new Board();
		board_from_fen(chessboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

		DrawBoard();
		GenerateMoves();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		DrawPieces();

		if (isPromotion) {
			DrawPromotion();
			if (promoted != 0) {
				isPromotion = false;
				for (const auto& move : possible[selected]) {
					if (move->to == selectedDestination && IS_PROMOTION(move->flags) && PROMOTED_PIECE(move->flags) == promoted) {
						if (IS_CAPTURE(move->flags)) {
							olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
						} else {
							olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
						}
						make_move(chessboard, move);
						switch_ply(chessboard);
						GenerateMoves();
						DrawBoard();

						// Highlight the source and destination squares of the most recently made move.
						FillRect({unit * (7 - selected % 8) + unit, unit * (7 - selected / 8) + unit}, {unit, unit}, highlight);
						FillRect({unit * (7 - selectedDestination % 8) + unit, unit * (7 - selectedDestination / 8) + unit}, {unit, unit}, highlight);
						break;
					}
				}
				promoted = 0;
			}
		} else if (GetMouse(0).bPressed) {
			olc::vf2d mouse = { (float) GetMouseX(), (float) GetMouseY() };
			int file = (mouse.x - unit) / unit;
			file = 7 - file;
			int rank = (mouse.y - unit) / unit;
			rank = 7 - rank;

			if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
				DrawBoard();
				DrawPieces();

				int index = rank * 8 + file;

				Move* to_make = nullptr;
				if (selected != -1) {
					for (const auto& move : possible[selected]) {
						if (move->to == index) {
							to_make = move;
							break;
						}
					}
				}

				if (to_make != nullptr) {
					selectedDestination = index;
					// Highlight the source and destination squares of the most recently made move.
					FillRect({unit * (7 - selected % 8) + unit, unit * (7 - selected / 8) + unit}, {unit, unit}, highlight);
					FillRect({unit * (7 - index % 8) + unit, unit * (7 - index / 8) + unit}, {unit, unit}, highlight);
					if (IS_CAPTURE(to_make->flags)) {
						olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
					} else {
						olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
					}

					if (IS_PROMOTION(to_make->flags)) {
						isPromotion = true;
					} else {
						make_move(chessboard, to_make);
						switch_ply(chessboard);
						GenerateMoves();
					}
				} else {
					selected = index;
					SetPixelMode(olc::Pixel::ALPHA);
					int prev = 0;
					for (const auto& move : possible[index]) {
						if (prev == move->to) continue; // Check for duplicates.
						prev = move->to;

						int r = 7 - move->to % 8;
						int f = 7 - move->to / 8;

						olc::vi2d center = {r * unit + unit * 3 / 2, f * unit + unit * 3 / 2};
						if (IS_CAPTURE(move->flags) && !IS_EN_PASSANT(move->flags)) {
							FillCircle(center, unit / 2.1, {0, 0, 0, 100});
							FillCircle(center, unit / 2.75, (r + f) % 2 != 0 ? dark : light);
						} else {
							FillCircle(center, unit / 5, {0, 0, 0, 100});
						}
					}
					SetPixelMode(olc::Pixel::NORMAL);
				}
			}
		}

		// if (GetKey(olc::Key::Q).bPressed) olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
		// if (GetKey(olc::Key::W).bPressed) olc::SOUND::PlaySample(audio[END_AUDIO]);
		// if (GetKey(olc::Key::E).bPressed) olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
		// if (GetKey(olc::Key::R).bPressed) olc::SOUND::PlaySample(audio[WARNING_AUDIO]);

		return true;
	}

	bool OnUserDestroy() {
		olc::SOUND::DestroyAudio();

		return true;
	}

	// Draw Screen for Pawn Promotions allowing user to selected promoted piece.
	void DrawPromotion() {
		int cx = screenWidth / 2;
		int cy = screenHeight / 2;
		int trim = unit / 5;
		int size = 4 * unit - 2 * trim;
		FillRect({cx - size / 2 - trim, cy - size / 2 - trim}, {size + 2 * trim, size + 2 * trim}, {0, 0, 0});
		FillRect({cx - size / 2, cy - size / 2}, {size, size}, {245, 245, 245});

		olc::vf2d scale = {(float) size / 2 / pieceSize, (float) size / 2 / pieceSize};

		olc::vf2d top_left = {(float) cx - size / 2, (float) cy - size / 2};
		olc::vf2d top_right = {(float) cx, (float) cy - size / 2};
		olc::vf2d bottom_left = {(float) cx - size / 2, (float) cy};
		olc::vf2d bottom_right = {(float) cx, (float) cy};

		olc::vf2d mouse = {(float) GetMouseX(), (float) GetMouseY()};

		auto overlap = [](olc::vf2d p1, olc::vf2d p2, int size) {
			return p2.x <= p1.x && p1.x <= p2.x + size && p2.y <= p1.y && p1.y <= p2.y + size;
		};

		bool tl = overlap(mouse, top_left, size / 2);
		bool tr = overlap(mouse, top_right, size / 2);
		bool bl = overlap(mouse, bottom_left, size / 2);
		bool br = overlap(mouse, bottom_right, size / 2);

		olc::vf2d sizevf = scale * pieceSize;

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
		}
	}

	// Draws the chess board.
	void DrawBoard() {
		Clear(background);
		int trim = unit / 4;
		FillRect({unit - trim, unit - trim}, {8 * unit + 2 * trim, 8 * unit + 2 * trim}, {0, 0, 0});

		// Draw Chess Board.
		for (int file = 0; file < 8; file++) {
			for (int rank = 0; rank < 8; rank++) {
				bool color = (file + rank) % 2 != 0;

				olc::vf2d pos = {(float) rank * unit + unit, (float) file * unit + unit};
				FillRect(pos, {unit, unit}, color ? dark : light);
			}
		}
	}

	// Draw the pieces on the board.
	void DrawPieces() {
		olc::vf2d scale = {(float) unit / pieceSize, (float) unit / pieceSize};

		if (isPromotion) {
			// If the promotion screen is up, then do not draw pieces on ranks 3-6 and files c-f.
			for (int file = 0; file < 8; file++) {
				for (int rank = 0; rank < 8; rank++) {
					if (file >= 2 && file <= 5 && rank >= 2 && rank <= 5) continue;
					int index = file * 8 + rank;
					Piece piece = get_piece(chessboard, index);
					if (piece != 0) {
						Piece color = get_color(chessboard, index);
						olc::vf2d pos = {(float) (7 - rank) * unit + unit, (float) (7 - file) * unit + unit};
						DrawDecal(pos, pieces[color][piece], scale);
					}
				}
			}
		} else {
			for (int file = 0; file < 8; file++) {
				for (int rank = 0; rank < 8; rank++) {
					int index = file * 8 + rank;
					Piece piece = get_piece(chessboard, index);
					if (piece != 0) {
						Piece color = get_color(chessboard, index);
						olc::vf2d pos = {(float) (7 - rank) * unit + unit, (float) (7 - file) * unit + unit};
						DrawDecal(pos, pieces[color][piece], scale);
					}
				}
			}
		}
	}

	void GenerateMoves() {
		for (int i = 0; i < 64; i++) {
			possible[i].clear();
		}

		int legal = gen_legal_moves(chessboard, moves);

		for (int i = 0; i < legal; i++) {
			Move* move = &moves[i];
			possible[move->from].push_back(move);
		}
	}
};

int main() {
	Chess demo;
	if (demo.Construct(800, 800, 1, 1))
		demo.Start();

	return 0;
}