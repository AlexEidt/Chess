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

	// Moves the player can make.
	Move moves[256];

	// Size of one square on the chess board.
	int unit;
	// Size of a chess piece sprite in pixels.
	float pieceSize;
	// Colors for the chess board.
	olc::Pixel light, dark, background;

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

			selected = -1;
		}

		srand(std::time(0));

		screenWidth = ScreenWidth();
		screenHeight = ScreenHeight();
		unit = std::min(screenWidth, screenHeight) / 10;
		pieceSize = pieces[WHITE][ROOK]->sprite->width;

		light = {235, 236, 208};
		dark = {119, 149, 86};
		background = {49, 46, 43};

		chessboard = new Board();
		board_from_fen(chessboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

		DrawBoard();

		makeMoves();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		DrawPieces();
		olc::vf2d mouse = { (float) GetMouseX(), (float) GetMouseY() };

		if (GetMouse(0).bPressed) {
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

				selected = index;

				if (to_make != nullptr) {
					if (IS_CAPTURE(to_make->flags)) {
						olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
					} else {
						olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
					}

					if (IS_PROMOTION(to_make->flags)) {
						
					}
					make_move(chessboard, to_make);
					switch_ply(chessboard);
					makeMoves();
				} else {
					for (const auto& move : possible[index]) {
						int r = 7 - move->to % 8;
						int f = 7 - move->to / 8;
						FillCircle({r * unit + unit * 3 / 2, f * unit + unit * 3 / 2}, unit / 4, {0, 0, 0, 100});
					}
				}
			}
		}

		if (GetKey(olc::Key::Q).bPressed) olc::SOUND::PlaySample(audio[CAPTURE_AUDIO]);
		if (GetKey(olc::Key::W).bPressed) olc::SOUND::PlaySample(audio[END_AUDIO]);
		if (GetKey(olc::Key::E).bPressed) olc::SOUND::PlaySample(audio[MOVE_AUDIO]);
		if (GetKey(olc::Key::R).bPressed) olc::SOUND::PlaySample(audio[WARNING_AUDIO]);

		return true;
	}

	bool OnUserDestroy() {
		olc::SOUND::DestroyAudio();

		return true;
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

	void makeMoves() {
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