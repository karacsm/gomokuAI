#include "gomoku.h"

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

ID2D1Bitmap* Assets::CROSS = 0;
ID2D1Bitmap* Assets::CIRCLE = 0;
ID2D1Bitmap* Assets::EMPTY_CELL = 0;
ID2D1Bitmap* Assets::PLAY_BUTTON = 0;
ID2D1Bitmap* Assets::PLAY_BUTTON_PRESSED = 0;
ID2D1Bitmap* Assets::EXIT_BUTTON = 0;
ID2D1Bitmap* Assets::EXIT_BUTTON_PRESSED = 0;
ID2D1Bitmap* Assets::MENU_BUTTON = 0;
ID2D1Bitmap* Assets::MENU_BUTTON_PRESSED = 0;
ID2D1Bitmap* Assets::MENU_BACKGROUND = 0;
ID2D1Bitmap* Assets::FILE_LABELS = 0;
ID2D1Bitmap* Assets::RANK_LABELS = 0;

int main() {

	ID2D1Factory* pFactory = NULL;
	ID2D1HwndRenderTarget* pRT = NULL;
	IWICImagingFactory* pIWICFactory = NULL;

	HINSTANCE hInstance = GetModuleHandle(0);
	const TCHAR windowClassName[] = _T("windowClass");
	const TCHAR windowTitle[] = _T("Gomoku AI");
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = windowClassName;
	wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
	wcex.hIcon = LoadIcon(hInstance, IDC_ICON);
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
	RegisterClassEx(&wcex);

	RECT wrect;
	wrect.left = 0;
	wrect.right = CLIENT_WIDTH;
	wrect.top = 0;
	wrect.bottom = CLIENT_HEIGHT;
	AdjustWindowRect(&wrect, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION, false);
	HWND hWnd = CreateWindow(windowClassName, windowTitle, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, wrect.right - wrect.left, wrect.bottom - wrect.top, 0, 0, hInstance, NULL);
	RECT crect;
	GetClientRect(hWnd, &crect);

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if (SUCCEEDED(hr)) {
		hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(crect.right - crect.left, crect.bottom - crect.top)), &pRT);
	}
	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
	}
	if (SUCCEEDED(hr)) {
		hr = loadAssets(pRT, pIWICFactory);
	}
	if (!SUCCEEDED(hr)) {
		return 1;
	}

	StateManager stateman;
	stateman.init("menu", new MenuState(&stateman));
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	Input input;
	input.escape_down = false;
	input.lb_down = false;
	input.rb_down = false;
	input.d_down = false;
	input.mouse_coord_x = 0;
	input.mouse_coord_y = 0;
	input.mouse_click_pos_x = 0;
	input.mouse_click_pos_y = 0;
	while (stateman.running()) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateman.stop();
			}
			handleInput(&input, msg.message, msg.wParam, msg.lParam);

		}
		POINT cursorpos;
		GetCursorPos(&cursorpos);
		ScreenToClient(hWnd, &cursorpos);
		input.mouse_coord_x = cursorpos.x;
		input.mouse_coord_y = cursorpos.y;

		stateman.update(input);

		pRT->BeginDraw();
		pRT->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

		stateman.render(pRT);

		hr = pRT->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET) {
			SafeRelease(&pRT);
			discardDeviceResources();
			pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(crect.right - crect.left, crect.bottom - crect.top)), &pRT);
			loadAssets(pRT, pIWICFactory);
		}
	}
	discardDeviceResources();
	SafeRelease(&pRT);
	SafeRelease(&pFactory);
	SafeRelease(&pIWICFactory);
	return 0;
}


void discardDeviceResources() {
	SafeRelease(&Assets::EMPTY_CELL);
	SafeRelease(&Assets::CROSS);
	SafeRelease(&Assets::CIRCLE);
	SafeRelease(&Assets::PLAY_BUTTON);
	SafeRelease(&Assets::PLAY_BUTTON_PRESSED);
	SafeRelease(&Assets::EXIT_BUTTON);
	SafeRelease(&Assets::EXIT_BUTTON_PRESSED);
	SafeRelease(&Assets::MENU_BUTTON);
	SafeRelease(&Assets::MENU_BUTTON_PRESSED);
	SafeRelease(&Assets::MENU_BACKGROUND);
	SafeRelease(&Assets::FILE_LABELS);
	SafeRelease(&Assets::RANK_LABELS);
}

HRESULT loadAssets(ID2D1HwndRenderTarget* pRT,IWICImagingFactory* pIWICFactory) {
	HRESULT hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/cell.png", TILE_SIZE, TILE_SIZE, &Assets::EMPTY_CELL);
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/cross.png", TILE_SIZE, TILE_SIZE, &Assets::CROSS);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/circle.png", TILE_SIZE, TILE_SIZE, &Assets::CIRCLE);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/play_button.png", 220, 63, &Assets::PLAY_BUTTON);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/play_button_pressed.png", 220, 63, &Assets::PLAY_BUTTON_PRESSED);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/exit_button.png", 220, 63, &Assets::EXIT_BUTTON);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/exit_button_pressed.png", 220, 63, &Assets::EXIT_BUTTON_PRESSED);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/menu_button.png", 220, 63, &Assets::MENU_BUTTON);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/menu_button_pressed.png", 220, 63, &Assets::MENU_BUTTON_PRESSED);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/menu.png", 800, 600, &Assets::MENU_BACKGROUND);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/files.png", 32 * 15, 32, &Assets::FILE_LABELS);
	}
	if (SUCCEEDED(hr)) {
		hr = LoadBitmapFromFile(pRT, pIWICFactory, L"resources/ranks.png", 32, 15 * 32, &Assets::RANK_LABELS);
	}
	return hr;
}
HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRT, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap) {
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;
	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr)) {
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr)) {
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr)) {
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr)) {
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}
	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);
	return hr;
}

void handleInput(Input* pInput, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_LBUTTONDOWN) {
		pInput->lb_down = true;
		pInput->mouse_click_pos_x = LOWORD(lParam);
		pInput->mouse_click_pos_y = HIWORD(lParam);
	}
	if (message == WM_LBUTTONUP) {
		pInput->lb_down = false;
		pInput->mouse_click_pos_x = LOWORD(lParam);
		pInput->mouse_click_pos_y = HIWORD(lParam);
	}
	if (message == WM_RBUTTONDOWN) {
		pInput->rb_down = true;
		pInput->mouse_click_pos_x = LOWORD(lParam);
		pInput->mouse_click_pos_y = HIWORD(lParam);
	}
	if (message == WM_RBUTTONUP) {
		pInput->rb_down = false;
		pInput->mouse_click_pos_x = LOWORD(lParam);
		pInput->mouse_click_pos_y = HIWORD(lParam);
	}
	if (message == WM_KEYDOWN && wParam == VK_ESCAPE) {
		pInput->escape_down = true;
	}
	if (message == WM_KEYUP && wParam == VK_ESCAPE) {
		pInput->escape_down = false;
	}
	//key: d, code: 0x44 
	if (message == WM_KEYDOWN && wParam == 0x44) {
		pInput->d_down = true;
	}
	if (message == WM_KEYUP && wParam == 0x44) {
		pInput->d_down = false;
	}
}

int mate_in(int n) {
	return MAX_SCORE - n;
}

//class Button
Button::Button(ID2D1Bitmap* image, ID2D1Bitmap* pressed_image, D2D1_RECT_F position, D2D1_RECT_F target, bool lb_active, bool rb_active):
	m_image(image), m_pressed_image(pressed_image), m_position(position), m_target(target), m_lb_pressed(false), m_rb_pressed(false), m_lb_active(lb_active), m_rb_active(rb_active) {
}

void Button::update(const Input &input) {
	if (input.lb_down) {
		if (input.mouse_click_pos_x >= m_target.left && input.mouse_click_pos_x < m_target.right && input.mouse_click_pos_y < m_target.bottom && input.mouse_click_pos_y >= m_target.top) {
			m_lb_pressed = true;
		}
	}
	else {
		m_lb_pressed = false;
	}

	if (input.rb_down) {
		if (input.mouse_click_pos_x >= m_target.left && input.mouse_click_pos_x < m_target.right && input.mouse_click_pos_y < m_target.bottom && input.mouse_click_pos_y >= m_target.top) {
			m_rb_pressed = true;
		}
	}
	else {
		m_rb_pressed = false;
	}
}
void Button::render(ID2D1HwndRenderTarget* pRT) {
	if ((m_lb_pressed && m_lb_active) || (m_rb_pressed && m_rb_active)) {
		if(m_pressed_image) pRT->DrawBitmap(m_pressed_image, m_position, 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	}
	else {
		if(m_image) pRT->DrawBitmap(m_image, m_position, 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	}
}
bool Button::lb_pressed() {
	return m_lb_pressed;
}

bool Button::rb_pressed() {
	return m_rb_pressed;
}

bool Button::lb_action(const Input& input) {
	if (!input.lb_down && m_lb_pressed && m_lb_active) {
		if (input.mouse_click_pos_x >= m_target.left && input.mouse_click_pos_x < m_target.right && input.mouse_click_pos_y < m_target.bottom && input.mouse_click_pos_y >= m_target.top) {
			return true;
		}
	}
	return false;
}

bool Button::rb_action(const Input& input) {
	if (!input.rb_down && m_rb_pressed && m_rb_active) {
		if (input.mouse_click_pos_x >= m_target.left && input.mouse_click_pos_x < m_target.right && input.mouse_click_pos_y < m_target.bottom && input.mouse_click_pos_y >= m_target.top) {
			return true;
		}
	}
	return false;
}

//class StateManager
StateManager::StateManager(): m_running(false), m_state_id("") {
}

StateManager::~StateManager() {
	for (std::map<std::string, State*>::iterator it = m_states.begin(); it != m_states.end(); it++) {
		if (it->second) delete it->second;
	}
}

void StateManager::init(std::string base_state_id, State* base_state) {
	m_state_id = base_state_id;
	if(base_state) base_state->start();
	m_states[base_state_id] = base_state;
	m_running = true;
}

void StateManager::add_state(std::string state_id, State* state) {
	m_states[state_id] = state;
}

void StateManager::remove_state(std::string state_id) {
	if (m_states.count(state_id)) {
		if (m_states[state_id]) delete m_states[state_id];
		m_states.erase(state_id);
	}
}

void StateManager::stop() {
	m_running = false;
}

void StateManager::switch_state(std::string state_id) {
	if (m_states.count(state_id)) {
		if (m_states[m_state_id]) m_states[m_state_id]->stop();
		if(m_states[state_id]) m_states[state_id]->start();
		m_state_id = state_id;
	}
}

bool StateManager::running() {
	return m_running;
}

void StateManager::update(const Input& input) {
	if(m_states[m_state_id]) m_states[m_state_id]->update(input);
}

void StateManager::render(ID2D1HwndRenderTarget* pRT) {
	if (m_states[m_state_id]) m_states[m_state_id]->render(pRT);
}


//class State
State::State(StateManager* state_manager):
	m_state_manager(state_manager) {
}

//class MenuState
MenuState::MenuState(StateManager* state_manager) : State(state_manager),
	m_play_button(Assets::PLAY_BUTTON, Assets::PLAY_BUTTON_PRESSED, D2D1::RectF(290, 200, 510, 263), D2D1::RectF(292, 202, 508, 261), true, false),
	m_exit_button(Assets::EXIT_BUTTON, Assets::EXIT_BUTTON_PRESSED, D2D1::RectF(290, 300, 510, 363), D2D1::RectF(292, 302, 508, 361), true, false),
	m_switch_button(0, 0, D2D1::RectF(384, 160, 416, 192), D2D1::RectF(384, 160, 416, 192), true, false) {
	m_ops.ai_to_move = true;
	m_ops.player_to_move = Player::X;
	m_ops.start_pos = "";
	m_ops.time_limit = 60000;
	m_ops.max_depth = 9;
}

void MenuState::update(const Input& input) {
	if (m_play_button.lb_action(input)) {
		m_state_manager->remove_state("game");
		m_state_manager->add_state("game", new GameState(m_state_manager, m_ops));
		m_state_manager->switch_state("game");
	}
	if (m_exit_button.lb_action(input)) {
		m_state_manager->stop();
	}

	if (m_switch_button.lb_action(input)) {
		m_ops.ai_to_move = !m_ops.ai_to_move;
	}

	m_play_button.update(input);
	m_exit_button.update(input);
	m_switch_button.update(input);
}

void MenuState::render(ID2D1HwndRenderTarget* pRT) {
	D2D1_SIZE_U client_size = pRT->GetPixelSize();
	if(Assets::MENU_BACKGROUND) pRT->DrawBitmap(Assets::MENU_BACKGROUND, D2D1::RectF(0, 0, float(client_size.width), float(client_size.height)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	if (m_ops.ai_to_move) {
		if (Assets::CROSS) pRT->DrawBitmap(Assets::CROSS, D2D1::RectF(384, 160, 416, 192), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	}
	else {
		if (Assets::CIRCLE) pRT->DrawBitmap(Assets::CIRCLE, D2D1::RectF(384, 160, 416, 192), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	}
	m_play_button.render(pRT);
	m_exit_button.render(pRT);
}

void MenuState::start() {
}

void MenuState::stop() {
}

//class Board
Board::Board(std::string start_pos):m_zobhash(0) {
	load_threats();
	load_table(m_threat_table, "threat_table.txt");

	std::mt19937 random_engine(674009171);
	std::uniform_int_distribution<uint64_t> dist;
	for (int i = 0; i < BOARD_DIM * BOARD_DIM * 2; i++) {
		m_zobnums.push_back(dist(random_engine));
	}

	for (int i = 0; i < BOARD_DIM * BOARD_DIM; i++) {
		m_legal_moves.insert(i);
	}
	for (int i = 0; i < BOARD_DIM; i++) {
		m_rows.push_back(std::string(15, 'e'));
		m_cols.push_back(std::string(15, 'e'));
	}
	for (int i = 0; i < BOARD_DIM; i++) {
		m_diag_pp.push_back(std::string(static_cast<size_t>(i) + 1, 'e'));
		m_diag_pm.push_back(std::string(size_t(i) + 1, 'e'));
	}
	for (int i = 0; i < BOARD_DIM - 1; i++) {
		m_diag_pp.push_back(std::string(static_cast<size_t>(BOARD_DIM) - static_cast<size_t>(i) - 1, 'e'));
		m_diag_pm.push_back(std::string(static_cast<size_t>(BOARD_DIM) - static_cast<size_t>(i) - 1, 'e'));
	}
	for (unsigned int i = 0; i < start_pos.size(); i++) {
		if (i >= 225) break;
		int col = i % BOARD_DIM;
		int row = (i - col) / BOARD_DIM;
		move(row, col, start_pos[i]);
	}
}

Board::~Board() {
}

bool Board::move(int row, int col, char p) {
	if (row >= 0 && row < BOARD_DIM && col >= 0 && col < BOARD_DIM && (p == 'x' || p == 'o') && m_legal_moves.count(row * BOARD_DIM + col)) {
		m_rows[row][col] = p;
		m_cols[col][row] = p;
		//diag pp
		int k = BOARD_DIM - 1 + col - row;
		int l = 0;
		if (k < BOARD_DIM) l = col; else l = row;
		m_diag_pp[k][l] = p;
		//diag pm
		k = row + col;
		if (k < BOARD_DIM) l = col; else l = BOARD_DIM - 1 - row;
		m_diag_pm[k][l] = p;
		//updating the set of legal moves
		m_legal_moves.erase(row * BOARD_DIM + col);
		//updating the of set moves
		m_moves.insert(row * BOARD_DIM + col);
		//updating hash
		if (p == 'x') {
			int index = 2 * (row * BOARD_DIM + col);
			m_zobhash = m_zobhash ^ m_zobnums[index];
		}
		else {
			int index = 2 * (row * BOARD_DIM + col) + 1;
			m_zobhash = m_zobhash ^ m_zobnums[index];
		}
		return true;
	}
	return false;
}

bool Board::move(int row, int col, Player player) {
	if (player == Player::X) {
		return move(row, col, 'x');
	}
	else if (player == Player::O) {
		return move(row, col, 'o');
	}
	else {
		return false;
	}
}

void Board::undo_move(int row, int col) {
	if (row >= 0 && row < BOARD_DIM && col >= 0 && col < BOARD_DIM && !m_legal_moves.count(row * BOARD_DIM + col)) {
		//updating hash
		if (at(row, col) == 'x') {
			int index = 2 * (row * BOARD_DIM + col);
			m_zobhash = m_zobhash ^ m_zobnums[index];
		}
		else {
			int index = 2 * (row * BOARD_DIM + col) + 1;
			m_zobhash = m_zobhash ^ m_zobnums[index];
		}
		char p = 'e';
		m_rows[row][col] = p;
		m_cols[col][row] = p;
		//diag pp
		int k = BOARD_DIM - 1 + col - row;
		int l = 0;
		if (k < BOARD_DIM) l = col; else l = row;
		m_diag_pp[k][l] = p;
		//diag pm
		k = row + col;
		if (k < BOARD_DIM) l = col; else l = BOARD_DIM - 1 - row;
		m_diag_pm[k][l] = p;
		//updating the set of legal moves
		m_legal_moves.insert(row * BOARD_DIM + col);
		//updating the set of moves
		m_moves.erase(row * BOARD_DIM + col);
	}
}

char Board::at(int row, int col) const {
	return m_rows[row][col];
}

Result Board::check_result(int last_move) const {
	int col = last_move % BOARD_DIM;
	int row = (last_move - col) / BOARD_DIM;
	char p = at(row, col);
	if (p == 'e') return Result::NONE;
	int k_pp = BOARD_DIM - 1 + col - row;
	int l_pp = 0;
	if (k_pp < BOARD_DIM) l_pp = col; else l_pp = row;
	int k_pm = row + col;
	int l_pm = 0;
	if (k_pm < BOARD_DIM) l_pm = col; else l_pm = BOARD_DIM - 1 - row;
	//check row
	int start = std::max<int>(0, col - 4);
	int end = std::min<int>(BOARD_DIM, col + 5);
	int combo = 0;
	for (int i = start; i < end; i++) {
		if (m_rows[row][i] == p) {
			combo++;
			if (combo >= 5) {
				if (p == 'x') return Result::XWIN;
				if (p == 'o') return Result::OWIN;
			}
		}
		else {
			combo = 0;
		}
	}
	//check column
	start = std::max<int>(0, row - 4);
	end = std::min<int>(BOARD_DIM, row + 5);
	combo = 0;
	for (int i = start; i < end; i++) {
		if (m_cols[col][i] == p) {
			combo++;
			if (combo >= 5) {
				if (p == 'x') return Result::XWIN;
				if (p == 'o') return Result::OWIN;
			}
		}
		else {
			combo = 0;
		}
	}
	//check diagonal pp
	start = std::max<int>(0, l_pp - 4);
	end = std::min<int>(static_cast<int>(m_diag_pp[k_pp].size()), l_pp + 5);
	combo = 0;
	for (int i = start; i < end; i++) {
		if (m_diag_pp[k_pp][i] == p) {
			combo++;
			if (combo >= 5) {
				if (p == 'x') return Result::XWIN;
				if (p == 'o') return Result::OWIN;
			}
		}
		else { 
			combo = 0;
		}
	}
	//chech diagonal pm
	start = std::max<int>(0, l_pm - 4);
	end = std::min<int>(static_cast<int>(m_diag_pm[k_pm].size()), l_pm + 5);
	combo = 0;
	for (int i = start; i < end; i++) {
		if (m_diag_pm[k_pm][i] == p) {
			combo++;
			if (combo >= 5) {
				if (p == 'x') return Result::XWIN;
				if (p == 'o') return Result::OWIN;
			}
		}
		else {
			combo = 0;
		}
	}
	if (m_legal_moves.size() == 0) return Result::DRAW;
	return Result::NONE;
}

std::unordered_set<int> Board::get_legal_moves() const {
	return m_legal_moves;
}

std::unordered_set<int> Board::get_boundary() const {
	std::unordered_set<int> boundary;
	std::vector<std::pair<int, int>> shifts;
	shifts.push_back(std::pair<int, int>(0, -2));
	shifts.push_back(std::pair<int, int>(0, -1));
	shifts.push_back(std::pair<int, int>(0, 1));
	shifts.push_back(std::pair<int, int>(0, 2));
	shifts.push_back(std::pair<int, int>(-2, 0));
	shifts.push_back(std::pair<int, int>(-1, 0));
	shifts.push_back(std::pair<int, int>(1, 0));
	shifts.push_back(std::pair<int, int>(2, 0));
	shifts.push_back(std::pair<int, int>(-2, -2));
	shifts.push_back(std::pair<int, int>(-1, -1));
	shifts.push_back(std::pair<int, int>(1, 1));
	shifts.push_back(std::pair<int, int>(2, 2));
	shifts.push_back(std::pair<int, int>(2, -2));
	shifts.push_back(std::pair<int, int>(1, -1));
	shifts.push_back(std::pair<int, int>(-1, 1));
	shifts.push_back(std::pair<int, int>(-2, 2));
	for (std::unordered_set<int>::iterator it = m_moves.begin(); it != m_moves.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		for (unsigned int i = 0; i < shifts.size(); i++) {
			int a = row + shifts[i].first;
			int b = col + shifts[i].second;
			if (m_legal_moves.count(a * BOARD_DIM + b) && a >= 0 && a < BOARD_DIM && b >=0 && b < BOARD_DIM) {
				boundary.insert(a * BOARD_DIM + b);
			}
		}
	}
	return boundary;
}

std::unordered_set<int> Board::get_small_boundary() const {
	std::unordered_set<int> boundary;
	std::vector<std::pair<int, int>> shifts;
	shifts.push_back(std::pair<int, int>(0, -1));
	shifts.push_back(std::pair<int, int>(0, 1));
	shifts.push_back(std::pair<int, int>(-1, 0));
	shifts.push_back(std::pair<int, int>(1, 0));
	shifts.push_back(std::pair<int, int>(-1, -1));
	shifts.push_back(std::pair<int, int>(1, 1));
	shifts.push_back(std::pair<int, int>(1, -1));
	shifts.push_back(std::pair<int, int>(-1, 1));
	for (std::unordered_set<int>::iterator it = m_moves.begin(); it != m_moves.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		for (unsigned int i = 0; i < shifts.size(); i++) {
			int a = row + shifts[i].first;
			int b = col + shifts[i].second;
			if (m_legal_moves.count(a * BOARD_DIM + b) && a >= 0 && a < BOARD_DIM && b >= 0 && b < BOARD_DIM) {
				boundary.insert(a * BOARD_DIM + b);
			}
		}
	}
	return boundary;
}

std::unordered_set<int> Board::get_building_boundary(Player player_to_move) const {
	std::unordered_set<int> boundary;
	std::vector<std::pair<int, int>> shifts;
	shifts.push_back(std::pair<int, int>(0, -1));
	shifts.push_back(std::pair<int, int>(0, 1));
	shifts.push_back(std::pair<int, int>(-1, 0));
	shifts.push_back(std::pair<int, int>(1, 0));
	shifts.push_back(std::pair<int, int>(-1, -1));
	shifts.push_back(std::pair<int, int>(1, 1));
	shifts.push_back(std::pair<int, int>(1, -1));
	shifts.push_back(std::pair<int, int>(-1, 1));
	for (std::unordered_set<int>::iterator it = m_moves.begin(); it != m_moves.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		for (unsigned int i = 0; i < shifts.size(); i++) {
			int a = row + shifts[i].first;
			int b = col + shifts[i].second;
			if (m_legal_moves.count(a * BOARD_DIM + b) && a >= 0 && a < BOARD_DIM && b >= 0 && b < BOARD_DIM) {
				bool building = false;
				for (unsigned int j = 0; j < shifts.size(); j++) {
					int aa = a + shifts[j].first;
					int bb = b + shifts[j].second;
					if (m_moves.count(aa * BOARD_DIM + bb) && aa >= 0 && aa < BOARD_DIM && bb >= 0 && bb < BOARD_DIM) {
						if ((player_to_move == Player::X && m_rows[aa][bb] == 'x') || (player_to_move == Player::O && m_rows[aa][bb] == 'o')) {
							building = true;
							break;
						}
					}
				}
				if (building) boundary.insert(a * BOARD_DIM + b);
			}
		}
	}
	return boundary;
}

std::vector<std::string> Board::get_rows() const {
	return m_rows;
}

std::vector<std::string> Board::get_cols() const {
	return m_cols;
}

std::vector<std::string> Board::get_diag_pp() const {
	return m_diag_pp;
}

std::vector<std::string> Board::get_diag_pm() const {
	return m_diag_pm;
}

std::pair<std::vector<Threat>, std::vector<Threat>> Board::get_threats() const {
	std::vector<Threat> threats_X;
	std::vector<Threat> threats_O;
	std::unordered_set<int> boundary = get_boundary();
	std::vector<std::string> rows_with_walls;
	std::vector<std::string> cols_with_walls;
	std::vector<std::string> diag_pp_with_walls;
	std::vector<std::string> diag_pm_with_walls;
	for (unsigned int i = 0; i < m_rows.size(); i++) {
		rows_with_walls.push_back("w" + m_rows[i] + "w");
	}
	for (unsigned int i = 0; i < m_cols.size(); i++) {
		cols_with_walls.push_back("w" + m_cols[i] + "w");
	}
	for (unsigned int i = 0; i < m_diag_pp.size(); i++) {
		diag_pp_with_walls.push_back("w" + m_diag_pp[i] + "w");
	}
	for (unsigned int i = 0; i < m_diag_pm.size(); i++) {
		diag_pm_with_walls.push_back("w" + m_diag_pm[i] + "w");
	}
	for (std::unordered_set<int>::iterator it = boundary.begin(); it != boundary.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		int k_pp = BOARD_DIM - 1 + col - row;
		int l_pp = 0;
		if (k_pp < BOARD_DIM) l_pp = col; else l_pp = row;
		int k_pm = row + col;
		int l_pm = 0;
		if (k_pm < BOARD_DIM) l_pm = col; else l_pm = BOARD_DIM - 1 - row;
		//check row
		int col_w = col + 1;
		int start = std::max<int>(0, col_w - 4);
		int end = std::min<int>(static_cast<int>(rows_with_walls[row].size()), col_w + 5);
		std::string line(rows_with_walls[row].begin() + start, rows_with_walls[row].begin() + end);
		line[col_w - start] = 'x';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threats[i].first].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threats[i].first].rest_squares[j] - 1;
						if (rest != col) threat.rest_squares.push_back(row * BOARD_DIM + rest);
						if (rest == col) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threats[i].first;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threats[i].first].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threats[i].first].cost_squares[j] - 1;
							threat.cost_squares.push_back(row * BOARD_DIM + cost);
						}
						threats_X.push_back(threat);
					}
				}
			}
		}
		line[col_w - start] = 'o';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first >= static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != col) threat.rest_squares.push_back(row * BOARD_DIM + rest);
						if (rest == col) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(row * BOARD_DIM + cost);
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check col
		int row_w = row + 1;
		start = std::max<int>(0, row_w - 4);
		end = std::min<int>(static_cast<int>(cols_with_walls[col].size()), row_w + 5);
		line = std::string(cols_with_walls[col].begin() + start, cols_with_walls[col].begin() + end);
		line[row_w - start] = 'x';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != row) threat.rest_squares.push_back(rest * BOARD_DIM + col);
						if (rest == row) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(cost* BOARD_DIM + col);
						}
						threats_X.push_back(threat);
					}
				}
			}
		}
		line[row_w - start] = 'o';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first >= static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != row) threat.rest_squares.push_back(rest * BOARD_DIM + col);
						if (rest == row) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(cost * BOARD_DIM + col);
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check diag_pp
		int l_pp_w = l_pp + 1;
		start = std::max<int>(0, l_pp_w - 4);
		end = std::min<int>(static_cast<int>(diag_pp_with_walls[k_pp].size()), l_pp_w + 5);
		line = std::string(diag_pp_with_walls[k_pp].begin() + start, diag_pp_with_walls[k_pp].begin() + end);
		line[l_pp_w - start] = 'x';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pp) threat.rest_squares.push_back(index_diag_pp(k_pp, rest));
						if (rest == l_pp) {
							valid = true;
							threat.gain_square = index_diag_pp(k_pp, l_pp);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pp(k_pp, cost));
						}
						threats_X.push_back(threat);
					}
				}
			}
		}
		line[l_pp_w - start] = 'o';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first >= static_cast<int>(m_threat_types_O.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pp) threat.rest_squares.push_back(index_diag_pp(k_pp, rest));
						if (rest == l_pp) {
							valid = true;
							threat.gain_square = index_diag_pp(k_pp, l_pp);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pp(k_pp, cost));
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check diag_pm
		int l_pm_w = l_pm + 1;
		start = std::max<int>(0, l_pm_w - 4);
		end = std::min<int>(static_cast<int>(diag_pm_with_walls[k_pm].size()), l_pm_w + 5);
		line = std::string(diag_pm_with_walls[k_pm].begin() + start, diag_pm_with_walls[k_pm].begin() + end);
		line[l_pm_w - start] = 'x';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pm) threat.rest_squares.push_back(index_diag_pm(k_pm, rest));
						if (rest == l_pm) {
							valid = true;
							threat.gain_square = index_diag_pm(k_pm, l_pm);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pm(k_pm, cost));
						}
						threats_X.push_back(threat);
					}
				}
			}
		}
		line[l_pm_w - start] = 'o';
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first >= static_cast<int>(m_threat_types_O.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pm) threat.rest_squares.push_back(index_diag_pm(k_pm, rest));
						if (rest == l_pm) {
							valid = true;
							threat.gain_square = index_diag_pm(k_pm, l_pm);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pm(k_pm, cost));
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
	}
	return std::pair<std::vector<Threat>, std::vector<Threat>>(threats_X, threats_O);
}
std::pair<std::vector<Threat>, std::vector<Threat>> Board::get_threats_played() const {
	std::vector<Threat> threats_X;
	std::vector<Threat> threats_O;
	std::vector<std::string> rows_with_walls;
	std::vector<std::string> cols_with_walls;
	std::vector<std::string> diag_pp_with_walls;
	std::vector<std::string> diag_pm_with_walls;
	for (unsigned int i = 0; i < m_rows.size(); i++) {
		rows_with_walls.push_back("w" + m_rows[i] + "w");
	}
	for (unsigned int i = 0; i < m_cols.size(); i++) {
		cols_with_walls.push_back("w" + m_cols[i] + "w");
	}
	for (unsigned int i = 0; i < m_diag_pp.size(); i++) {
		diag_pp_with_walls.push_back("w" + m_diag_pp[i] + "w");
	}
	for (unsigned int i = 0; i < m_diag_pm.size(); i++) {
		diag_pm_with_walls.push_back("w" + m_diag_pm[i] + "w");
	}
	for (std::unordered_set<int>::iterator it = m_moves.begin(); it != m_moves.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		int k_pp = BOARD_DIM - 1 + col - row;
		int l_pp = 0;
		if (k_pp < BOARD_DIM) l_pp = col; else l_pp = row;
		int k_pm = row + col;
		int l_pm = 0;
		if (k_pm < BOARD_DIM) l_pm = col; else l_pm = BOARD_DIM - 1 - row;
		//check row
		int col_w = col + 1;
		int start = std::max<int>(0, col_w - 4);
		int end = std::min<int>(static_cast<int>(rows_with_walls[row].size()), col_w + 5);
		std::string line(rows_with_walls[row].begin() + start, rows_with_walls[row].begin() + end);
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threats[i].first].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threats[i].first].rest_squares[j] - 1;
						if (rest != col) threat.rest_squares.push_back(row * BOARD_DIM + rest);
						if (rest == col) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threats[i].first;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threats[i].first].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threats[i].first].cost_squares[j] - 1;
							threat.cost_squares.push_back(row * BOARD_DIM + cost);
						}
						threats_X.push_back(threat);
					}
				}
				if (threats[i].first >= static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != col) threat.rest_squares.push_back(row * BOARD_DIM + rest);
						if (rest == col) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(row * BOARD_DIM + cost);
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check col
		int row_w = row + 1;
		start = std::max<int>(0, row_w - 4);
		end = std::min<int>(static_cast<int>(cols_with_walls[col].size()), row_w + 5);
		line = std::string(cols_with_walls[col].begin() + start, cols_with_walls[col].begin() + end);
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != row) threat.rest_squares.push_back(rest * BOARD_DIM + col);
						if (rest == row) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(cost * BOARD_DIM + col);
						}
						threats_X.push_back(threat);
					}
				}
				if (threats[i].first >= static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != row) threat.rest_squares.push_back(rest * BOARD_DIM + col);
						if (rest == row) {
							valid = true;
							threat.gain_square = row * BOARD_DIM + col;
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(cost * BOARD_DIM + col);
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check diag_pp
		int l_pp_w = l_pp + 1;
		start = std::max<int>(0, l_pp_w - 4);
		end = std::min<int>(static_cast<int>(diag_pp_with_walls[k_pp].size()), l_pp_w + 5);
		line = std::string(diag_pp_with_walls[k_pp].begin() + start, diag_pp_with_walls[k_pp].begin() + end);
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pp) threat.rest_squares.push_back(index_diag_pp(k_pp, rest));
						if (rest == l_pp) {
							valid = true;
							threat.gain_square = index_diag_pp(k_pp, l_pp);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pp(k_pp, cost));
						}
						threats_X.push_back(threat);
					}
				}
				if (threats[i].first >= static_cast<int>(m_threat_types_O.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pp) threat.rest_squares.push_back(index_diag_pp(k_pp, rest));
						if (rest == l_pp) {
							valid = true;
							threat.gain_square = index_diag_pp(k_pp, l_pp);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pp(k_pp, cost));
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
		//check diag_pm
		int l_pm_w = l_pm + 1;
		start = std::max<int>(0, l_pm_w - 4);
		end = std::min<int>(static_cast<int>(diag_pm_with_walls[k_pm].size()), l_pm_w + 5);
		line = std::string(diag_pm_with_walls[k_pm].begin() + start, diag_pm_with_walls[k_pm].begin() + end);
		if (m_threat_table.count(line)) {
			std::vector<std::pair<int, int>> threats = m_threat_table.at(line);
			for (unsigned int i = 0; i < threats.size(); i++) {
				if (threats[i].first < static_cast<int>(m_threat_types_X.size())) {
					int threat_type_index = threats[i].first;
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_X[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pm) threat.rest_squares.push_back(index_diag_pm(k_pm, rest));
						if (rest == l_pm) {
							valid = true;
							threat.gain_square = index_diag_pm(k_pm, l_pm);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_X[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_X[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pm(k_pm, cost));
						}
						threats_X.push_back(threat);
					}
				}
				if (threats[i].first >= static_cast<int>(m_threat_types_O.size())) {
					int threat_type_index = threats[i].first - static_cast<int>(m_threat_types_X.size());
					Threat threat;
					bool valid = false;
					for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].rest_squares.size(); j++) {
						int rest = start + threats[i].second + m_threat_types_O[threat_type_index].rest_squares[j] - 1;
						if (rest != l_pm) threat.rest_squares.push_back(index_diag_pm(k_pm, rest));
						if (rest == l_pm) {
							valid = true;
							threat.gain_square = index_diag_pm(k_pm, l_pm);
							threat.threat_type_index = threat_type_index;
						}
					}
					if (valid) {
						for (unsigned int j = 0; j < m_threat_types_O[threat_type_index].cost_squares.size(); j++) {
							int cost = start + threats[i].second + m_threat_types_O[threat_type_index].cost_squares[j] - 1;
							threat.cost_squares.push_back(index_diag_pm(k_pm, cost));
						}
						threats_O.push_back(threat);
					}
				}
			}
		}
	}
	return std::pair<std::vector<Threat>, std::vector<Threat>>(threats_X, threats_O);
}

uint64_t Board::get_hash() const {
	return m_zobhash;
}

std::vector<int> Board::get_searchspace(Player player_to_move) const {
	std::pair<std::vector<Threat>, std::vector<Threat>> threats = get_threats();
	std::pair<std::vector<Threat>, std::vector<Threat>> threats_played = get_threats_played();
	std::vector<Threat> five_threats_X;
	std::vector<Threat> five_threats_O;
	std::vector<Threat> open_four_threats_X;
	std::vector<Threat> open_four_threats_O;
	std::vector<Threat> four_threats_X;
	std::vector<Threat> four_threats_O;
	std::vector<Threat> three_threats_X;
	std::vector<Threat> three_threats_O;
	std::vector<Threat> open_four_threats_played_X;
	std::vector<Threat> open_four_threats_played_O;
	std::vector<Threat> four_threats_played_X;
	std::vector<Threat> four_threats_played_O;
	std::vector<Threat> three_threats_played_X;
	std::vector<Threat> three_threats_played_O;
	for (unsigned int i = 0; i < threats.first.size(); i++) {
		if (threats.first[i].threat_type_index == 0) five_threats_X.push_back(threats.first[i]);
		if (threats.first[i].threat_type_index == 1) open_four_threats_X.push_back(threats.first[i]);
		if (threats.first[i].threat_type_index >= 2 && threats.first[i].threat_type_index < 7) four_threats_X.push_back(threats.first[i]);
		if (threats.first[i].threat_type_index >= 7) three_threats_X.push_back(threats.first[i]);
	}
	for (unsigned int i = 0; i < threats.second.size(); i++) {
		if (threats.second[i].threat_type_index == 0) five_threats_O.push_back(threats.second[i]);
		if (threats.second[i].threat_type_index == 1) open_four_threats_O.push_back(threats.second[i]);
		if (threats.second[i].threat_type_index >= 2 && threats.second[i].threat_type_index < 7) four_threats_O.push_back(threats.second[i]);
		if (threats.second[i].threat_type_index >= 7) three_threats_O.push_back(threats.second[i]);
	}
	for (unsigned int i = 0; i < threats_played.first.size(); i++) {
		if (threats_played.first[i].threat_type_index == 1) open_four_threats_played_X.push_back(threats_played.first[i]);
		if (threats_played.first[i].threat_type_index >= 2 && threats_played.first[i].threat_type_index < 7) four_threats_played_X.push_back(threats_played.first[i]);
		if (threats_played.first[i].threat_type_index >= 7) three_threats_played_X.push_back(threats_played.first[i]);
	}
	for (unsigned int i = 0; i < threats_played.second.size(); i++) {
		if (threats_played.second[i].threat_type_index == 1) open_four_threats_played_O.push_back(threats_played.second[i]);
		if (threats_played.second[i].threat_type_index >= 2 && threats_played.second[i].threat_type_index < 7) four_threats_played_O.push_back(threats_played.second[i]);
		if (threats_played.second[i].threat_type_index >= 7) three_threats_played_O.push_back(threats_played.second[i]);
	}
	if (player_to_move == Player::X && five_threats_X.size()) {
		std::vector<int> searchspace;
		searchspace.push_back(five_threats_X[0].gain_square);
		return searchspace;
	}
	else if (player_to_move == Player::O && five_threats_O.size()) {
		std::vector<int> searchspace;
		searchspace.push_back(five_threats_O[0].gain_square);
		return searchspace;
	}
	else if (player_to_move == Player::X && four_threats_played_O.size()) {
		return four_threats_played_O[0].cost_squares;
	}
	else if (player_to_move == Player::X && open_four_threats_played_O.size()) {
		return open_four_threats_played_O[0].cost_squares;
	}
	else if (player_to_move == Player::O && four_threats_played_X.size()) {
		return four_threats_played_X[0].cost_squares;
	}
	else if (player_to_move == Player::O && open_four_threats_played_X.size()) {
		return open_four_threats_played_X[0].cost_squares;
	}
	else if (player_to_move == Player::X && three_threats_played_O.size()) {
		std::unordered_set<int> searchspace;
		for (unsigned int i = 0; i < three_threats_played_O[0].cost_squares.size(); i++) {
			searchspace.insert(three_threats_played_O[0].cost_squares[i]);
		}
		for (unsigned int i = 0; i < four_threats_X.size(); i++) {
			searchspace.insert(four_threats_X[i].gain_square);
		}
		for (unsigned int i = 0; i < open_four_threats_X.size(); i++) {
			searchspace.insert(open_four_threats_X[i].gain_square);
		}
		return std::vector<int>(searchspace.begin(), searchspace.end());
	}
	else if (player_to_move == Player::O && three_threats_played_X.size()) {
		std::unordered_set<int> searchspace;
		for (unsigned int i = 0; i < three_threats_played_X[0].cost_squares.size(); i++) {
			searchspace.insert(three_threats_played_X[0].cost_squares[i]);
		}
		for (unsigned int i = 0; i < four_threats_O.size(); i++) {
			searchspace.insert(four_threats_O[i].gain_square);
		}
		for (unsigned int i = 0; i < open_four_threats_O.size(); i++) {
			searchspace.insert(open_four_threats_O[i].gain_square);
		}
		return std::vector<int>(searchspace.begin(), searchspace.end());
	}
	else {
		std::unordered_set<int> threatspace;
		for (unsigned int i = 0; i < threats.first.size(); i++) {
			threatspace.insert(threats.first[i].gain_square);
		}
		for (unsigned int i = 0; i < threats.second.size(); i++) {
			threatspace.insert(threats.second[i].gain_square);
		}
		std::vector<int> searchspace(threatspace.begin(), threatspace.end());
		std::unordered_set<int> boundary = get_building_boundary(player_to_move);
		for (std::unordered_set<int>::iterator it = boundary.begin(); it != boundary.end(); it++) {
			if (!threatspace.count(*it)) searchspace.push_back(*it);
		}
		if (searchspace.size() == 0) {
			std::unordered_set<int> small_boundary = get_small_boundary();
			return std::vector<int>(small_boundary.begin(), small_boundary.end());
		}
		return searchspace;
	}
}

//X -> +, O -> -
int Board::evaluate(Player player_to_move) const {
	std::pair<std::vector<Threat>, std::vector<Threat>> threats = get_threats();
	//sw, initiative, attacking mobility
	bool four_X = false;
	bool open_four_X = false;
	bool three_X = false;
	bool four_O = false;
	bool open_four_O = false;
	bool three_O = false;
	std::unordered_set<int> gain_squares_X;
	std::unordered_set<int> gain_squares_O;
	for (unsigned int i = 0; i < threats.first.size(); i++) {
		if (threats.first[i].threat_type_index == 0) {
			four_X = true;
			int start_col = threats.first[i].rest_squares[0] % BOARD_DIM;
			int start_row = (threats.first[i].rest_squares[0] - start_col) / BOARD_DIM;
			int end_col = threats.first[i].rest_squares[3] % BOARD_DIM;
			int end_row = (threats.first[i].rest_squares[3] - end_col) / BOARD_DIM;
			int shift_row = (end_row - start_row);
			if (shift_row != 0) shift_row /= std::abs(shift_row);
			int shift_col = (end_col - start_col);
			if (shift_col != 0) shift_col /= std::abs(shift_col);
			if ((end_row - start_row) == 3 * shift_row && (end_col - start_col) == 3 * shift_col) {
				if ((start_row - shift_row) >= 0 && (start_row - shift_row) < BOARD_DIM && (start_col - shift_col) >= 0 && (start_col - shift_col) < BOARD_DIM) {
					if ((end_row + shift_row) >= 0 && (end_row + shift_row < BOARD_DIM) && (end_col + shift_col) >= 0 && (end_col + shift_col) < BOARD_DIM) {
						if (at(start_row - shift_row, start_col - shift_col) == 'e' && at(end_row + shift_row, end_col + shift_col) == 'e') {
							open_four_X = true;
						}
					}
				}
			}
		}
		if (threats.first[i].threat_type_index == 1) {
			three_X = true;
		}
		gain_squares_X.insert(threats.first[i].gain_square);
	}
	for (unsigned int i = 0; i < threats.second.size(); i++) {
		if (threats.second[i].threat_type_index == 0) {
			four_O = true;
			int start_col = threats.second[i].rest_squares[0] % BOARD_DIM;
			int start_row = (threats.second[i].rest_squares[0] - start_col) / BOARD_DIM;
			int end_col = threats.second[i].rest_squares[3] % BOARD_DIM;
			int end_row = (threats.second[i].rest_squares[3] - end_col) / BOARD_DIM;
			int shift_row = (end_row - start_row);
			if (shift_row != 0) shift_row /= std::abs(shift_row);
			int shift_col = (end_col - start_col);
			if (shift_col != 0) shift_col /= std::abs(shift_col);
			if ((end_row - start_row) == 3 * shift_row && (end_col - start_col) == 3 * shift_col) {
				if ((start_row - shift_row) >= 0 && (start_row - shift_row) < BOARD_DIM && (start_col - shift_col) >= 0 && (start_col - shift_col) < BOARD_DIM) {
					if ((end_row + shift_row) >= 0 && (end_row + shift_row < BOARD_DIM) && (end_col + shift_col) >= 0 && (end_col + shift_col) < BOARD_DIM) {
						if (at(start_row - shift_row, start_col - shift_col) == 'e' && at(end_row + shift_row, end_col + shift_col) == 'e') {
							open_four_O = true;
						}
					}
				}
			}
		}
		if (threats.second[i].threat_type_index == 1) {
			three_O = true;
		}
		gain_squares_O.insert(threats.second[i].gain_square);
	}
	//sw
	if (player_to_move == Player::X) {
		if (four_X) return mate_in(1);
		if (three_X && !four_O) return mate_in(3);

		if (open_four_O) return -mate_in(2);
	}

	if (player_to_move == Player::O) {
		if (four_O) return mate_in(1);
		if (three_O && !four_X) return mate_in(3);

		if (open_four_X) return -mate_in(2);
	}
	//initiative and attacking mobility
	Player initiative = Player::EMPTY;
	int gain_X = static_cast<int>(gain_squares_X.size());
	int gain_O = static_cast<int>(gain_squares_O.size());
	if (player_to_move == Player::X) {
		if (four_O) initiative = Player::O;
		if (!four_O && !three_O && gain_X) initiative = Player::X;
		if (!four_O && three_O && three_X) initiative = Player::X;
	}
	if (player_to_move == Player::O) {
		if (four_X) initiative = Player::X;
		if (!four_X && !three_X && gain_O) initiative = Player::O;
		if (!four_X && three_X && three_O) initiative = Player::O;
	}
	float score = 0;
	if (initiative == Player::EMPTY) {
		score = float(gain_X - gain_O);
	}
	if (initiative == Player::X) {
		score =  INITIATIVE_MULTYPLIER * float(gain_X) - float(gain_O);
	}
	if (initiative == Player::O) {
		score =  float(gain_X) - INITIATIVE_MULTYPLIER * float(gain_O);
	}
	if (player_to_move == Player::X) return static_cast<int>(score * 10);
	else return -static_cast<int>(score * 10);
}

std::string Board::get_position() const {
	std::string pos;
	for (int i = 0; i < BOARD_DIM; i++) {
		for (int j = 0; j < BOARD_DIM; j++) {
			pos += m_rows[i][j];
		}
	}
	return pos;
}

std::vector<int> Board::search_pattern(std::string line, std::regex pattern) const {
	std::vector<int> pos;
	std::string suffix = line;
	std::smatch sm;
	int offset = 0;
	while (std::regex_search(suffix, sm, pattern)) {
		pos.push_back(static_cast<int>(sm.position() + offset));
		offset += static_cast<int>(sm.prefix().length()) + 1;
		suffix = std::string(line.begin() + offset, line.end());
	}
	return pos;
}

void Board::load_threats() {
	Threat_type five_X;
	five_X.pattern = std::regex("xxxxx", std::regex_constants::optimize);
	five_X.rest_squares.push_back(0); 
	five_X.rest_squares.push_back(1);
	five_X.rest_squares.push_back(2);
	five_X.rest_squares.push_back(3);
	five_X.rest_squares.push_back(4);
	m_threat_types_X.push_back(five_X);
	Threat_type open_four_X;
	open_four_X.pattern = std::regex("exxxxe", std::regex_constants::optimize);
	open_four_X.rest_squares.push_back(1);
	open_four_X.rest_squares.push_back(2);
	open_four_X.rest_squares.push_back(3);
	open_four_X.rest_squares.push_back(4);
	open_four_X.cost_squares.push_back(0);
	m_threat_types_X.push_back(open_four_X);
	Threat_type four0_left_X;
	four0_left_X.pattern = std::regex("exxxxo|exxxxw", std::regex_constants::optimize);
	four0_left_X.rest_squares.push_back(1);
	four0_left_X.rest_squares.push_back(2);
	four0_left_X.rest_squares.push_back(3);
	four0_left_X.rest_squares.push_back(4);
	four0_left_X.cost_squares.push_back(0);
	m_threat_types_X.push_back(four0_left_X);
	Threat_type four0_right_X;
	four0_right_X.pattern = std::regex("oxxxxe|wxxxxe", std::regex_constants::optimize);
	four0_right_X.rest_squares.push_back(1);
	four0_right_X.rest_squares.push_back(2);
	four0_right_X.rest_squares.push_back(3);
	four0_right_X.rest_squares.push_back(4);
	four0_right_X.cost_squares.push_back(5);
	m_threat_types_X.push_back(four0_right_X);
	Threat_type four1_left_X;
	four1_left_X.pattern = std::regex("xexxx", std::regex_constants::optimize);
	four1_left_X.rest_squares.push_back(0);
	four1_left_X.rest_squares.push_back(2);
	four1_left_X.rest_squares.push_back(3);
	four1_left_X.rest_squares.push_back(4);
	four1_left_X.cost_squares.push_back(1);
	m_threat_types_X.push_back(four1_left_X);
	Threat_type four1_right_X;
	four1_right_X.pattern = std::regex("xxxex", std::regex_constants::optimize);
	four1_right_X.rest_squares.push_back(0);
	four1_right_X.rest_squares.push_back(1);
	four1_right_X.rest_squares.push_back(2);
	four1_right_X.rest_squares.push_back(4);
	four1_right_X.cost_squares.push_back(3);
	m_threat_types_X.push_back(four1_right_X);
	Threat_type four2_X;
	four2_X.pattern = std::regex("xxexx", std::regex_constants::optimize);
	four2_X.rest_squares.push_back(0);
	four2_X.rest_squares.push_back(1);
	four2_X.rest_squares.push_back(3);
	four2_X.rest_squares.push_back(4);
	four2_X.cost_squares.push_back(2);
	m_threat_types_X.push_back(four2_X);
	Threat_type open_three_X;
	open_three_X.pattern = std::regex("eexxxee", std::regex_constants::optimize);
	open_three_X.rest_squares.push_back(2);
	open_three_X.rest_squares.push_back(3);
	open_three_X.rest_squares.push_back(4);
	open_three_X.cost_squares.push_back(1);
	open_three_X.cost_squares.push_back(5);
	m_threat_types_X.push_back(open_three_X);
	Threat_type three_left_X;
	three_left_X.pattern = std::regex("eexxxeo|eexxxew", std::regex_constants::optimize);
	three_left_X.rest_squares.push_back(2);
	three_left_X.rest_squares.push_back(3);
	three_left_X.rest_squares.push_back(4);
	three_left_X.cost_squares.push_back(0);
	three_left_X.cost_squares.push_back(1);
	three_left_X.cost_squares.push_back(5);
	m_threat_types_X.push_back(three_left_X);
	Threat_type three_right_X;
	three_right_X.pattern = std::regex("oexxxee|wexxxee", std::regex_constants::optimize);
	three_right_X.rest_squares.push_back(2);
	three_right_X.rest_squares.push_back(3);
	three_right_X.rest_squares.push_back(4);
	three_right_X.cost_squares.push_back(1);
	three_right_X.cost_squares.push_back(5);
	three_right_X.cost_squares.push_back(6);
	m_threat_types_X.push_back(three_right_X);
	Threat_type broken_three_left_X;
	broken_three_left_X.pattern = std::regex("exxexe", std::regex_constants::optimize);
	broken_three_left_X.rest_squares.push_back(1);
	broken_three_left_X.rest_squares.push_back(2);
	broken_three_left_X.rest_squares.push_back(4);
	broken_three_left_X.cost_squares.push_back(0);
	broken_three_left_X.cost_squares.push_back(3);
	broken_three_left_X.cost_squares.push_back(5);
	m_threat_types_X.push_back(broken_three_left_X);
	Threat_type broken_three_right_X;
	broken_three_right_X.pattern = std::regex("exexxe", std::regex_constants::optimize);
	broken_three_right_X.rest_squares.push_back(1);
	broken_three_right_X.rest_squares.push_back(3);
	broken_three_right_X.rest_squares.push_back(4);
	broken_three_right_X.cost_squares.push_back(0);
	broken_three_right_X.cost_squares.push_back(2);
	broken_three_right_X.cost_squares.push_back(5);
	m_threat_types_X.push_back(broken_three_right_X);
	//O
	Threat_type five_O;
	five_O.pattern = std::regex("ooooo", std::regex_constants::optimize);
	five_O.rest_squares.push_back(0);
	five_O.rest_squares.push_back(1);
	five_O.rest_squares.push_back(2);
	five_O.rest_squares.push_back(3);
	five_O.rest_squares.push_back(4);
	m_threat_types_O.push_back(five_O);
	Threat_type open_four_O;
	open_four_O.pattern = std::regex("eooooe", std::regex_constants::optimize);
	open_four_O.rest_squares.push_back(1);
	open_four_O.rest_squares.push_back(2);
	open_four_O.rest_squares.push_back(3);
	open_four_O.rest_squares.push_back(4);
	open_four_O.cost_squares.push_back(0);
	m_threat_types_O.push_back(open_four_O);
	Threat_type four0_left_O;
	four0_left_O.pattern = std::regex("eoooox|eoooow", std::regex_constants::optimize);
	four0_left_O.rest_squares.push_back(1);
	four0_left_O.rest_squares.push_back(2);
	four0_left_O.rest_squares.push_back(3);
	four0_left_O.rest_squares.push_back(4);
	four0_left_O.cost_squares.push_back(0);
	m_threat_types_O.push_back(four0_left_O);
	Threat_type four0_right_O;
	four0_right_O.pattern = std::regex("xooooe|wooooe", std::regex_constants::optimize);
	four0_right_O.rest_squares.push_back(1);
	four0_right_O.rest_squares.push_back(2);
	four0_right_O.rest_squares.push_back(3);
	four0_right_O.rest_squares.push_back(4);
	four0_right_O.cost_squares.push_back(5);
	m_threat_types_O.push_back(four0_right_O);
	Threat_type four1_left_O;
	four1_left_O.pattern = std::regex("oeooo", std::regex_constants::optimize);
	four1_left_O.rest_squares.push_back(0);
	four1_left_O.rest_squares.push_back(2);
	four1_left_O.rest_squares.push_back(3);
	four1_left_O.rest_squares.push_back(4);
	four1_left_O.cost_squares.push_back(1);
	m_threat_types_O.push_back(four1_left_O);
	Threat_type four1_right_O;
	four1_right_O.pattern = std::regex("oooeo", std::regex_constants::optimize);
	four1_right_O.rest_squares.push_back(0);
	four1_right_O.rest_squares.push_back(1);
	four1_right_O.rest_squares.push_back(2);
	four1_right_O.rest_squares.push_back(4);
	four1_right_O.cost_squares.push_back(3);
	m_threat_types_O.push_back(four1_right_O);
	Threat_type four2_O;
	four2_O.pattern = std::regex("ooeoo", std::regex_constants::optimize);
	four2_O.rest_squares.push_back(0);
	four2_O.rest_squares.push_back(1);
	four2_O.rest_squares.push_back(3);
	four2_O.rest_squares.push_back(4);
	four2_O.cost_squares.push_back(2);
	m_threat_types_O.push_back(four2_O);
	Threat_type open_three_O;
	open_three_O.pattern = std::regex("eeoooee", std::regex_constants::optimize);
	open_three_O.rest_squares.push_back(2);
	open_three_O.rest_squares.push_back(3);
	open_three_O.rest_squares.push_back(4);
	open_three_O.cost_squares.push_back(1);
	open_three_O.cost_squares.push_back(5);
	m_threat_types_O.push_back(open_three_O);
	Threat_type three_left_O;
	three_left_O.pattern = std::regex("eeoooex|eeoooew", std::regex_constants::optimize);
	three_left_O.rest_squares.push_back(2);
	three_left_O.rest_squares.push_back(3);
	three_left_O.rest_squares.push_back(4);
	three_left_O.cost_squares.push_back(0);
	three_left_O.cost_squares.push_back(1);
	three_left_O.cost_squares.push_back(5);
	m_threat_types_O.push_back(three_left_O);
	Threat_type three_right_O;
	three_right_O.pattern = std::regex("xeoooee|weoooee", std::regex_constants::optimize);
	three_right_O.rest_squares.push_back(2);
	three_right_O.rest_squares.push_back(3);
	three_right_O.rest_squares.push_back(4);
	three_right_O.cost_squares.push_back(1);
	three_right_O.cost_squares.push_back(5);
	three_right_O.cost_squares.push_back(6);
	m_threat_types_O.push_back(three_right_O);
	Threat_type broken_three_left_O;
	broken_three_left_O.pattern = std::regex("eooeoe", std::regex_constants::optimize);
	broken_three_left_O.rest_squares.push_back(1);
	broken_three_left_O.rest_squares.push_back(2);
	broken_three_left_O.rest_squares.push_back(4);
	broken_three_left_O.cost_squares.push_back(0);
	broken_three_left_O.cost_squares.push_back(3);
	broken_three_left_O.cost_squares.push_back(5);
	m_threat_types_O.push_back(broken_three_left_O);
	Threat_type broken_three_right_O;
	broken_three_right_O.pattern = std::regex("eoeooe", std::regex_constants::optimize);
	broken_three_right_O.rest_squares.push_back(1);
	broken_three_right_O.rest_squares.push_back(3);
	broken_three_right_O.rest_squares.push_back(4);
	broken_three_right_O.cost_squares.push_back(0);
	broken_three_right_O.cost_squares.push_back(2);
	broken_three_right_O.cost_squares.push_back(5);
	m_threat_types_O.push_back(broken_three_right_O);
}

void Board::load_table(std::unordered_map<std::string, std::vector<std::pair<int, int>>>& table, std::string filename) {
	std::ifstream file;
	file.open(filename);
	std::string key;
	while (file >> key) {
		int size = 0;
		file >> size;
		std::vector<std::pair<int, int>> threats;
		for (int i = 0; i < size; i++) {
			int type = 0;
			int pos = 0;
			file >> type;
			file >> pos;
			threats.push_back(std::pair<int, int>(type, pos));
		}
		table[key] = threats;
	}
	file.close();
}

int Board::index_diag_pp(int k, int l) const {
	if (k < BOARD_DIM) {
		int col = l;
		int row = col + BOARD_DIM - 1 - k;
		return row * BOARD_DIM + col;
	}
	else {
		int row = l;
		int col = row + k - BOARD_DIM + 1;
		return row * BOARD_DIM + col;
	}
}
int Board::index_diag_pm(int k, int l) const {
	if (k < BOARD_DIM) {
		int col = l;
		int row = k - col;
		return row * BOARD_DIM + col;
	}
	else {
		int row = BOARD_DIM - 1 - l;
		int col = k - row;
		return row * BOARD_DIM + col;
	}
}
//class GameState
GameState::GameState(StateManager* state_manager, GameStateOptions ops) : State(state_manager), m_ops(ops), m_ai(Player::O), m_ai_to_move(false),
m_menu_button(Assets::MENU_BUTTON, Assets::MENU_BUTTON_PRESSED, D2D1::RectF(688, 2, 798, 33), D2D1::RectF(688, 2, 798, 33), true, false),
m_board_button(0, 0, D2D1::RectF(OFFSET_X, OFFSET_Y, OFFSET_X + TILE_SIZE * BOARD_DIM, OFFSET_Y + TILE_SIZE * BOARD_DIM), D2D1::RectF(OFFSET_X, OFFSET_Y, OFFSET_X + TILE_SIZE * BOARD_DIM, OFFSET_Y + TILE_SIZE * BOARD_DIM), true, true),
m_game_over(false), m_board(ops.start_pos), m_last_move(0), m_thinking(false), m_ai_thread(0), m_exiting(false), m_tt(0), m_count(0) {
	m_tt_size = 1048576; // 2^20
	m_tt = new TTEntry[m_tt_size];
	if (ops.ai_to_move) {
		m_ai = ops.player_to_move;
		if (m_board.get_legal_moves().size() == 225) {
			m_board.move(7, 7, m_ai);
			m_ai_to_move = false;
		}
		else {
			m_ai_to_move = true;
		}
	}
	else {
		m_ai_to_move = false;
		if (ops.player_to_move == Player::X) m_ai = Player::O;
		if (ops.player_to_move == Player::O) m_ai = Player::X;
	}
	std::cout << "It's your turn. Click to make a move." << std::endl;
}

GameState::~GameState() {
	m_exiting = true;
	if (m_ai_thread) {
		if (m_ai_thread->joinable()) {
			m_ai_thread->join();
		}
		delete m_ai_thread;
	}
	if (m_tt) {
		delete[] m_tt;
	}
}

void GameState::update(const Input& input) {
	if (m_menu_button.lb_action(input)) {
		m_exiting = true;
		if (m_ai_thread) {
			if (m_ai_thread->joinable()) {
				m_ai_thread->join();
			}
		}
		m_state_manager->switch_state("menu");
		m_state_manager->remove_state("game");
	}
	if (m_ai_to_move && !m_game_over && !m_thinking) {
		m_thinking = true;
		std::cout << "Thinking . . ." << std::endl;
		if (m_ai_thread) {
			if (m_ai_thread->joinable()) {
				m_ai_thread->join();
				delete m_ai_thread;
				m_ai_thread = new std::thread(&GameState::generate_move, this, m_ops.max_depth, 10000);
			}
		}
		else {
			m_ai_thread = new std::thread(&GameState::generate_move, this, m_ops.max_depth, 10000);
		}
	}
	if (m_board_button.lb_action(input) && !m_ai_to_move && !m_game_over) {
		int row = (input.mouse_click_pos_y - static_cast<int>(OFFSET_Y)) / TILE_SIZE;
		int col = (input.mouse_click_pos_x - static_cast<int>(OFFSET_X)) / TILE_SIZE;
		bool moved = m_board.move(row, col, switch_player(m_ai));
		if (moved) {
			m_last_move = row * BOARD_DIM + col;
			Result r = m_board.check_result(row * BOARD_DIM + col);
			if (r == Result::XWIN) std::cout << "X won" << std::endl;
			if (r == Result::OWIN) std::cout << "O won" << std::endl;
			if (r != Result::NONE) m_game_over = true;
			m_ai_to_move = true;
		}
	}
	m_menu_button.update(input);
	m_board_button.update(input);
}

void GameState::render(ID2D1HwndRenderTarget* pRT) {
	ID2D1SolidColorBrush* pYellowBrush = 0;
	ID2D1SolidColorBrush* pRedBrush = 0;
	ID2D1SolidColorBrush* pBlueBrush = 0;
	ID2D1SolidColorBrush* pGreenBrush = 0;
	pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0.0f, 0.8f), &pYellowBrush);
	pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f, 0.8f), &pRedBrush);
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 1.0f, 0.8f), &pBlueBrush);
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.0f, 1.0f, 0.0f, 0.2f), &pGreenBrush);
	m_menu_button.render(pRT);
	if (Assets::RANK_LABELS) pRT->DrawBitmap(Assets::RANK_LABELS, D2D1::RectF(OFFSET_X - TILE_SIZE, OFFSET_Y, OFFSET_X, OFFSET_Y + BOARD_DIM * TILE_SIZE), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	if (Assets::RANK_LABELS) pRT->DrawBitmap(Assets::RANK_LABELS, D2D1::RectF(OFFSET_X + BOARD_DIM * TILE_SIZE, OFFSET_Y, OFFSET_X + (BOARD_DIM + 1)* TILE_SIZE, OFFSET_Y + BOARD_DIM * TILE_SIZE), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	if (Assets::FILE_LABELS) pRT->DrawBitmap(Assets::FILE_LABELS, D2D1::RectF(OFFSET_X, OFFSET_Y - TILE_SIZE, OFFSET_X + BOARD_DIM * TILE_SIZE, OFFSET_Y), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	if (Assets::FILE_LABELS) pRT->DrawBitmap(Assets::FILE_LABELS, D2D1::RectF(OFFSET_X, OFFSET_Y + TILE_SIZE * BOARD_DIM, OFFSET_X + BOARD_DIM * TILE_SIZE, OFFSET_Y + TILE_SIZE * (BOARD_DIM + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	for (int i = 0; i < BOARD_DIM; i++) {
		for (int j = 0; j < BOARD_DIM; j++) {
			if (m_board.at(i, j) == 'x') {
				if (Assets::CROSS) pRT->DrawBitmap(Assets::CROSS, D2D1::RectF(OFFSET_X + j * TILE_SIZE, OFFSET_Y + i * TILE_SIZE, OFFSET_X + (j + 1) * TILE_SIZE, OFFSET_Y + (i + 1) * TILE_SIZE), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			}
			if (m_board.at(i, j) == 'o') {
				if (Assets::CIRCLE) pRT->DrawBitmap(Assets::CIRCLE, D2D1::RectF(OFFSET_X + j * TILE_SIZE, OFFSET_Y + i * TILE_SIZE, OFFSET_X + (j + 1) * TILE_SIZE, OFFSET_Y + (i + 1) * TILE_SIZE), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			}
			if(m_board.at(i, j) == 'e') {
				if (Assets::EMPTY_CELL) pRT->DrawBitmap(Assets::EMPTY_CELL, D2D1::RectF(OFFSET_X + j * TILE_SIZE, OFFSET_Y + i * TILE_SIZE, OFFSET_X + (j + 1) * TILE_SIZE, OFFSET_Y + (i + 1) * TILE_SIZE), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			}
		}
	}
	/*std::unordered_set<int> boundary = m_board.get_boundary();
	for (std::unordered_set<int>::iterator it = boundary.begin(); it != boundary.end(); it++) {
		int col = (*it) % BOARD_DIM;
		int row = ((*it) - col) / BOARD_DIM;
		if(pYellowBrush) pRT->FillRectangle(D2D1::RectF(OFFSET_X + col * TILE_SIZE + 2, OFFSET_Y + row * TILE_SIZE + 2, OFFSET_X + (col + 1) * TILE_SIZE - 2, OFFSET_Y + (row + 1) * TILE_SIZE - 2), pYellowBrush);
	}*/
	int col = m_last_move % BOARD_DIM;
	int row = (m_last_move - col) / BOARD_DIM;
	if (pGreenBrush) pRT->FillRectangle(D2D1::RectF(OFFSET_X + col * TILE_SIZE + 2, OFFSET_Y + row * TILE_SIZE + 2, OFFSET_X + (col + 1) * TILE_SIZE - 2, OFFSET_Y + (row + 1) * TILE_SIZE - 2), pGreenBrush);
	std::pair<std::vector<Threat>, std::vector<Threat>> threats;// = m_board.get_threats();
	/*for (unsigned int i = 0; i < threats.first.size(); i++) {
		int col = threats.first[i].gain_square % BOARD_DIM;
		int row = (threats.first[i].gain_square - col) / BOARD_DIM;
		if (pRedBrush) pRT->FillRectangle(D2D1::RectF(OFFSET_X + col * TILE_SIZE + 2, OFFSET_Y + row * TILE_SIZE + 2, OFFSET_X + (col + 1) * TILE_SIZE - 2, OFFSET_Y + (row + 1) * TILE_SIZE - 2), pRedBrush);
	}*/
	/*for (unsigned int i = 0; i < threats.second.size(); i++) {
		int col = threats.second[i].gain_square % BOARD_DIM;
		int row = (threats.second[i].gain_square  - col) / BOARD_DIM;
		if (pRedBrush) pRT->FillRectangle(D2D1::RectF(OFFSET_X + col * TILE_SIZE + 2, OFFSET_Y + row * TILE_SIZE + 2, OFFSET_X + (col + 1) * TILE_SIZE - 2, OFFSET_Y + (row + 1) * TILE_SIZE - 2), pRedBrush);
	}*/
	SafeRelease(&pYellowBrush);
	SafeRelease(&pRedBrush);
	SafeRelease(&pBlueBrush);
	SafeRelease(&pGreenBrush);
}

void GameState::start() {
}

void GameState::stop() {
}

int GameState::alphabeta(Board& board, Node* node, int alpha, int beta, int depth, bool* failed, const std::chrono::steady_clock::time_point& start_time) {
	if (m_exiting || (*failed)) return 0;
	uint64_t key = board.get_hash();
	TTEntry* ptt = probeHash(key);
	if ((ptt->depth >= depth) && (ptt->key == key)) {
		if (ptt->type == TTEntryType::EXACT) {
			return ptt->value;
		}
		if ((ptt->type == TTEntryType::ALPHA) && (ptt->value <= alpha)) {
			return ptt->value;
		}
		if ((ptt->type == TTEntryType::BETA) && (ptt->value >= beta)) {
			return ptt->value;
		}
	}
	m_count++;
	if (m_count > 1000) {
		m_count = 0;
		std::chrono::steady_clock clock;
		std::chrono::steady_clock::time_point now = clock.now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > m_ops.time_limit) {
			*failed = true;
			return 0;
		}
	}
	Result result = board.check_result(node->move);
	if (result == Result::DRAW) {
		save_entry(key, depth, 0, TTEntryType::EXACT);
		return 0;
	}
	if (result == Result::XWIN) {
		if (node->player_to_move == Player::X) {
			save_entry(key, depth, mate_in(0), TTEntryType::EXACT);
			return mate_in(0);
		}
		else {
			save_entry(key, depth, -mate_in(0), TTEntryType::EXACT);
			return -mate_in(0);
		}
	}
	if (result == Result::OWIN) {
		if (node->player_to_move == Player::X) {
			save_entry(key, depth, -mate_in(0), TTEntryType::EXACT);
			return -mate_in(0);
		}
		else {
			save_entry(key, depth, mate_in(0), TTEntryType::EXACT);
			return mate_in(0);
		}
	}
	if (depth <= 0) {
		int value = board.evaluate(node->player_to_move);
		save_entry(key, depth, value, TTEntryType::EXACT);
		return value;
	}
	if (node->child_nodes.size() == 0) {
		std::vector<int> moves = board.get_searchspace(node->player_to_move);
		for (unsigned int i = 0; i < moves.size(); i++) {
			node->child_nodes.push_back(new Node(moves[i], switch_player(node->player_to_move)));
		}
	}
	int best_value = -MAX_SCORE;
	TTEntryType type = TTEntryType::ALPHA;
	for (unsigned int i = 0; (i < node->child_nodes.size()); i++) {
		int col = node->child_nodes[i]->move % BOARD_DIM;
		int row = (node->child_nodes[i]->move - col) / BOARD_DIM;
		board.move(row, col, node->player_to_move);
		int value = -alphabeta(board, node->child_nodes[i], -beta, -alpha, depth - 1, failed, start_time);
		board.undo_move(row, col);
		if (*failed) return 0;
		else {
			node->child_nodes[i]->value = value;
			if (node->child_nodes[i]->value > 10000) node->child_nodes[i]->value--;
			if (node->child_nodes[i]->value < -10000) node->child_nodes[i]->value++;
		}

		if (node->child_nodes[i]->value > alpha) {
			alpha = node->child_nodes[i]->value;
			type = TTEntryType::EXACT;
		}
		if (node->child_nodes[i]->value > best_value) best_value = node->child_nodes[i]->value;
		if (alpha >= beta) {
			save_entry(key, depth, best_value, TTEntryType::BETA);
			return best_value;
		}
	}
	save_entry(key, depth, best_value, type);
	return best_value;
}

Player GameState::switch_player(Player player) {
	if (player == Player::X) {
		return Player::O;
	}
	if (player == Player::O) {
		return Player::X;
	}
	return Player::EMPTY;
}

void GameState::generate_move(int max_depth, int time_limit) {
	std::chrono::steady_clock clock;
	std::chrono::steady_clock::time_point start = clock.now();
	Board board(m_board.get_position());
	Node root(m_last_move, m_ai);
	int depth_reached = 0;
	bool failed = false;
	for (int i = 1; i <= max_depth; i++) {
		if ((std::abs(root.value) > 10000) && i > 1) {
			int value = -alphabeta(board, &root, -std::abs(root.value), std::abs(root.value), i, &failed, start);
			if (!failed) root.value = value;
		}
		else {
			int value = -alphabeta(board, &root, -MAX_SCORE, MAX_SCORE, i, &failed, start);
			if (!failed) root.value = value;
		}
		std::chrono::steady_clock::time_point end = clock.now();
		if (!failed && (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <= time_limit)) {
			root.sort_nodes();
			depth_reached++;
		}
		else {
			if (!failed) depth_reached++;
			break;
		}
	}
	if (m_exiting) return;
	root.sort_nodes();
	int move = root.child_nodes[0]->move;
	int col = move % BOARD_DIM;
	int row = (move - col) / BOARD_DIM;
	bool moved = m_board.move(row, col, m_ai);
	m_last_move = move;
	if (failed) std::cout << "failed" << std::endl;
	std::cout << "depth: " << depth_reached << " ply" << std::endl;
	std::cout << "value: " << root.value << std::endl;
	Result r = m_board.check_result(move);
	if (r == Result::XWIN) std::cout << "X won" << std::endl;
	if (r == Result::OWIN) std::cout << "O won" << std::endl;
	if (r != Result::NONE) m_game_over = true;
	else std::cout << "I made my move, it is your turn." << std::endl;
	std::cout << std::endl;
	m_ai_to_move = false;
	m_thinking = false;
}

TTEntry* GameState::probeHash(uint64_t key) {
	return &m_tt[key % m_tt_size];
}

void GameState::save_entry(uint64_t key, int depth, int value, TTEntryType type) {
	uint64_t index = key % m_tt_size;
	m_tt[index].key = key;
	m_tt[index].depth = depth;
	m_tt[index].value = value;
	m_tt[index].type = type;
}


Node::Node(const Node& node):move(node.move), value(node.value), player_to_move(node.player_to_move) {
	for (unsigned int i = 0; i < node.child_nodes.size(); i++) {
		child_nodes.push_back(new Node(*(node.child_nodes[i])));
	}
}

Node::~Node() {
	for (unsigned int i = 0; i < child_nodes.size(); i++) {
		if (child_nodes[i]) delete child_nodes[i];
	}
}

void Node::sort_nodes() {
	std::sort(child_nodes.begin(), child_nodes.end(), Node_greater());
	for (unsigned int i = 0; i < child_nodes.size(); i++) {
		child_nodes[i]->sort_nodes();
	}
}
