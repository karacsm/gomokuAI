#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <thread>

#include <windows.h>
#include <wincodec.h>
#include <stdlib.h>
#include <tchar.h>
#include <regex>
#include <random>

#include "d2d1.h"

template<class T> void SafeRelease(T** ppInterface) {
	if (*ppInterface) {
		(*ppInterface)->Release();
		*ppInterface = NULL;
	}
}

struct Input;
class Button;
class StateManager;
class State;
class MenuState;
enum class Player;
enum class Result;
struct GameStateOptions;
class GameState;
class Board;
struct Threat_type;
struct Threat;
struct Node;

const int CLIENT_WIDTH = 800;
const int CLIENT_HEIGHT = 600;
const int TILE_SIZE = 32;
const int BOARD_DIM = 15;

const float OFFSET_X = float(CLIENT_WIDTH - BOARD_DIM * TILE_SIZE) / 2;
const float OFFSET_Y = float(CLIENT_HEIGHT - BOARD_DIM * TILE_SIZE) / 2;
const int MAX_SCORE = 100000;
const float INITIATIVE_MULTYPLIER = 1.2f;

std::vector<Threat_type> THREAT_TYPES_X;
std::vector<Threat_type> THREAT_TYPES_O;

HRESULT loadAssets(ID2D1HwndRenderTarget* pRT, IWICImagingFactory* pIWICFactory);

void discardDeviceResources();
HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRT, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);
void handleInput(Input* pInput, UINT message, WPARAM wParam, LPARAM lParam);

int mate_in(int n);

struct Input {

	bool lb_down;
	bool rb_down;
	bool d_down;
	bool escape_down;

	int mouse_coord_x;
	int mouse_coord_y;

	int mouse_click_pos_x;
	int mouse_click_pos_y;
};

namespace Assets {
	extern ID2D1Bitmap* EMPTY_CELL;
	extern ID2D1Bitmap* CROSS;
	extern ID2D1Bitmap* CIRCLE;
	extern ID2D1Bitmap* PLAY_BUTTON;
	extern ID2D1Bitmap* PLAY_BUTTON_PRESSED;
	extern ID2D1Bitmap* EXIT_BUTTON;
	extern ID2D1Bitmap* EXIT_BUTTON_PRESSED;
	extern ID2D1Bitmap* MENU_BUTTON;
	extern ID2D1Bitmap* MENU_BUTTON_PRESSED;
	extern ID2D1Bitmap* MENU_BACKGROUND;
	extern ID2D1Bitmap* FILE_LABELS;
	extern ID2D1Bitmap* RANK_LABELS;
};

class Button {
private:
	ID2D1Bitmap* m_image;
	ID2D1Bitmap* m_pressed_image;
	bool m_lb_pressed;
	bool m_rb_pressed;
	bool m_lb_active;
	bool m_rb_active;
	D2D1_RECT_F m_position;
	D2D1_RECT_F m_target;
public:
	Button(ID2D1Bitmap* image, ID2D1Bitmap* pressed_image, D2D1_RECT_F position, D2D1_RECT_F target, bool lb_active, bool rb_active);
	void update(const Input &input);
	void render(ID2D1HwndRenderTarget* pRT);
	bool lb_pressed();
	bool rb_pressed();
	bool lb_action(const Input &input);
	bool rb_action(const Input& input);
};

class StateManager {
private:
	std::map<std::string, State*> m_states;
	bool m_running;
	std::string m_state_id;
public:
	StateManager();
	~StateManager();
	void init(std::string base_state_id, State* base_state);
	void add_state(std::string state_id, State* state);
	void remove_state(std::string state_id);
	void stop();
	void switch_state(std::string state_id);
	bool running();
	void update(const Input& input);
	void render(ID2D1HwndRenderTarget* pRT);
};

class State {
protected:
	StateManager* m_state_manager;
public:
	virtual ~State() {}
	virtual void update(const Input& input) = 0;
	virtual void render(ID2D1HwndRenderTarget* pRT) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
	State(StateManager* state_manager);
};

enum class Player {
	X, O, EMPTY
};

struct GameStateOptions {
	std::string start_pos;
	bool ai_to_move;
	Player player_to_move;
	int time_limit; //milliseconds
	int max_depth;
	GameStateOptions() :ai_to_move(true), time_limit(10000), player_to_move(Player::X), max_depth(3) {};
};

class MenuState : public State {
private:
	Button m_play_button;
	Button m_exit_button;
	Button m_switch_button;
	GameStateOptions m_ops;
public:
	MenuState(StateManager* state_manager);
	void update(const Input& input);
	void render(ID2D1HwndRenderTarget* pRT);
	void start();
	void stop();
};

enum class Result {
	XWIN, OWIN, DRAW, NONE
};

//defines a threat type
struct Threat_type {
	std::regex pattern;
	std::vector<int> rest_squares;//relative index
	std::vector<int> cost_squares;//relative index
};

//realization of threat
struct Threat {
	int gain_square;
	int threat_type_index;
	std::vector<int> rest_squares;
	std::vector<int> cost_squares;
	Threat() : gain_square(0), threat_type_index(0) {}
};

enum class TTEntryType {
	EMPTY, EXACT, ALPHA, BETA
};

struct TTEntry {
	uint64_t key;
	int depth;
	int value;
	TTEntryType type;
	TTEntry():key(0), depth(0), value(0), type(TTEntryType::EMPTY){}
};

class Board {
private:
	uint64_t m_zobhash;
	std::vector<std::string> m_rows;//row representation of the board
	std::vector<std::string> m_cols;//column representation of the board
	std::vector<std::string> m_diag_pp;// "\" diagonal representation of the board
	std::vector<std::string> m_diag_pm;// "/" diagonal representation of the board
	std::unordered_set<int> m_legal_moves;//remaining legal moves
	std::unordered_set<int> m_moves;//moves already played
	std::vector<Threat_type> m_threat_types_X;
	std::vector<Threat_type> m_threat_types_O;
	std::unordered_map<std::string, std::vector<std::pair<int, int>>> m_threat_table;
	std::vector<uint64_t> m_zobnums;
public:
	Board(std::string start_pos);
	~Board();
	bool move(int row, int col, char p);
	bool move(int row, int col, Player player);
	void undo_move(int row, int col);
	char at(int row, int col) const;
	Result check_result(int last_move) const;
	std::unordered_set<int> get_legal_moves() const;
	std::unordered_set<int> get_boundary() const;
	std::unordered_set<int> get_small_boundary() const;
	std::unordered_set<int> get_building_boundary(Player player_to_move) const;
	std::vector<std::string> get_rows() const;
	std::vector<std::string> get_cols() const;
	std::vector<std::string> get_diag_pp() const;
	std::vector<std::string> get_diag_pm() const;
	std::pair<std::vector<Threat>, std::vector<Threat>> get_threats() const;
	std::pair<std::vector<Threat>, std::vector<Threat>> get_threats_played() const;

	uint64_t get_hash() const;
	std::vector<int> get_searchspace(Player player_to_move) const;
	int evaluate(Player player_to_move) const; //for negamax
	std::string get_position() const;
	
private:
	std::vector<int> search_pattern(std::string line, std::regex pattern) const;
	void load_threats();
	void load_table(std::unordered_map<std::string, std::vector<std::pair<int, int>>> &table, std::string filename);
	int index_diag_pp(int k, int l) const;
	int index_diag_pm(int k, int l) const;
};

struct Node {
	Player player_to_move;
	int value;
	int move;
	std::vector<Node*> child_nodes;
	Node(int move, Player player_to_move) : move(move), value(-MAX_SCORE), player_to_move(player_to_move) {}
	Node(const Node& node);
	~Node();
	void sort_nodes();
};

struct Node_greater {
	bool operator()(const Node* n1, const Node* n2) {
		if (n1 && n2) return n1->value > n2->value;
		else return false;
	}
};

struct Node_less {
	bool operator()(const Node* n1, const Node* n2) {
		if (n1 && n2) return n1->value < n2->value;
		else return false;
	}
};

class GameState : public State {
private:
	GameStateOptions m_ops;
	bool m_ai_to_move;
	Player m_ai;
	Board m_board;
	Button m_menu_button;
	Button m_board_button;
	bool m_game_over;
	int m_last_move;
	bool m_thinking;
	bool m_exiting;
	std::thread* m_ai_thread;
	uint64_t m_tt_size;
	TTEntry* m_tt;
	int m_count;
public:
	GameState(StateManager* state_manager, GameStateOptions ops);
	~GameState();
	void update(const Input& input);
	void render(ID2D1HwndRenderTarget* pRT);
	void start();
	void stop();
private:
	int alphabeta(Board& board, Node* node, int alpha, int beta, int depth, bool* failed, const std::chrono::steady_clock::time_point& start_time); //negamax with move ordering
	Player switch_player(Player player);
	void generate_move(int max_depth, int time);
	TTEntry* probeHash(uint64_t key);
	void save_entry(uint64_t key, int depth, int value, TTEntryType type);
};