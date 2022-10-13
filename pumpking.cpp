// Alex Eidt
// Pumpking Chess Engine.

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

extern "C" {
	#include "Pumpking/board.h"
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

	// Size of one square on the chess board.
	int unit;
	// Dark and Light colors for the chess board.
	olc::Pixel dark, light;

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

		dark = {91, 156, 254};
		light = {220, 238, 255};

		Clear(dark);

		// Draw Chess Board.
		for (int file = 0; file < 8; file++) {
			for (int rank = 0; rank < 8; rank++) {
				bool color = (file + rank) % 2 != 0;

				olc::vf2d pos = {(float) rank * unit + unit, (float) file * unit + unit};
				FillRect(pos, {unit, unit}, color ? dark : light);
			}
		}

		from_fen(chessboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		DrawBoard();

		olc::vf2d mouse = { (float) GetMouseX(), (float) GetMouseY() };

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

	// Draw the pieces on the board.
	void DrawBoard() {
		// All chess piece sprites have the same dimensions.
		float pieceSize = pieces[WHITE][ROOK]->sprite->width;
		olc::vf2d scale = {(float) unit / pieceSize, (float) unit / pieceSize};
		for (int file = 0; file < 8; file++) {
			for (int rank = 0; rank < 8; rank++) {
				int index = file * 8 + rank;
				Piece piece = get_piece(chessboard, index);
				Piece color = get_color(chessboard, index);
				if (piece != 0) {
					olc::vf2d pos = {(float) rank * unit + unit, (float) file * unit + unit};
					DrawDecal(pos, pieces[color][piece], scale);
				}
			}
		}
	}
};

int main() {
	Chess demo;
	if (demo.Construct(500, 500, 1, 1))
		demo.Start();

	return 0;
}