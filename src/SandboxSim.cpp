#include "raylib.h"
#include <vector>
#include <string>
#include <algorithm>

#include "Graphics/DebugOverlay.h"

// --- CONFIGURATION SETTINGS ---
const int SCREEN_WIDTH = 800;   //! The width of the application window in pixels
const int SCREEN_HEIGHT = 600;  //! The height of the application window in pixels
//! DON'T CHANGE FOR NOW!!
//? 800x600 is ok?
const int UI_HEIGHT = 80;       //! Height of the bottom UI panel (for buttons)
const int SIM_HEIGHT_PIXELS = SCREEN_HEIGHT - UI_HEIGHT; //! The actual play area height
const int SCALE = 4;            //! Scaling factor: 1 simulation cell = 4x4 screen pixels (Optimization)

//! Calculated dimensions of the simulation grid based on the scale
const int SIM_WIDTH = SCREEN_WIDTH / SCALE;
const int SIM_HEIGHT = SIM_HEIGHT_PIXELS / SCALE;

// --- ELEMENT TYPES (ENUMS) ---
//! Instead of checking for '1', we check for 'SAND'.
enum ElementType {
	EMPTY = 0,
	SAND = 1,
	WATER = 2,
	WALL = 3,
	WOOD = 4,
	FIRE = 5,
	SMOKE = 6,
	ACID = 7,
	ACIDIC_WATER = 8
};

// --- DATA STRUCTURE FOR ELEMENTS ---
//! Holds the properties of each element type for easy UI generation and rendering.
struct ElementDef {
	int id;             //! The ID matching the Enum
	std::string name;   //! Name to display in the UI/Inspector
	Color color;        //! The base color of the element
};

//! List of all available elements.
std::vector<ElementDef> elements = {
	{ SAND, "SAND",  GOLD },
	{ WATER, "WATER", SKYBLUE },
	{ WALL, "WALL",  DARKGRAY },
	{ WOOD, "WOOD",  BROWN },
	{ FIRE, "FIRE",  ORANGE },
	{ SMOKE, "SMOKE", { 150, 150, 150, 180 } }, //! Gray with transparency
	{ ACID, "ACID",  LIME },
	{ ACIDIC_WATER, "A.WATER", { 0, 240, 200, 200 } }, //! Turquoise
	{ EMPTY, "ERASE", BLACK }
	//TODO More Elements...
};

//! Helper function: Returns the color associated with an element ID
Color GetColor(int id) {
	for (const auto& e : elements) if (e.id == id) return e.color;
	return BLACK; //! Default fallback
}

//! Helper function: Returns the name string of an element ID
std::string GetName(int id) {
	for (const auto& e : elements) if (e.id == id) return e.name;
	return "EMPTY";
}

// --- SIMULATION GRIDS ---
//! We use a 1D vector to represent a 2D grid for better memory performance (Cache Locality).
//! 'grid': Represents the current frame.
//! 'nextGrid': Represents the state of the next frame.
std::vector<int> grid(SIM_WIDTH* SIM_HEIGHT, EMPTY);
std::vector<int> nextGrid(SIM_WIDTH* SIM_HEIGHT, EMPTY);

// Tools and Input state
int currentTool = SAND; //! Currently selected element to draw
int brushSize = 3;      //! Size of the drawing brush

//! Helper: Checks if an index is within the valid range of the vector
bool IsValid(int index) {
	return index >= 0 && index < (int)grid.size();
}

// --- MAIN PHYSICS ENGINE ---
void UpdateSimulation() {
	//! Start by copying the current state to the next state.
	//! This ensures that static objects (Walls, stationary Sand) stay where they are.
	nextGrid = grid;

	// --- PASS 1: BOTTOM-TO-TOP SCAN ---
	//! We scan from the bottom up for gravity-affected elements (Sand, Water).
	//! If we scanned top-down, a particle would fall through the entire screen in one frame.
	for (int y = SIM_HEIGHT - 1; y >= 0; y--) {
		for (int x = 0; x < SIM_WIDTH; x++) {

			//! To prevent bias (always flowing left), we alternate the horizontal scan direction.
			//! If row is even -> scan left-to-right. If odd -> scan right-to-left.
			int scanX = (y % 2 == 0) ? x : (SIM_WIDTH - 1 - x);

			//! Convert 2D coordinates (x, y) to 1D index (i)
			int i = y * SIM_WIDTH + scanX;
			int type = grid[i];

			//! Optimization: Skip empty cells or static/gas elements in this pass
			if (type == EMPTY || type == WALL || type == WOOD || type == SMOKE || type == FIRE) continue;

			//! Calculate indices of neighbors
			int below = i + SIM_WIDTH;
			int belowL = i + SIM_WIDTH - 1;
			int belowR = i + SIM_WIDTH + 1;
			int left = i - 1;
			int right = i + 1;

			bool moved = false; //! Flag to track if the particle moved this frame

			// --- LOGIC FOR LIQUIDS (Water, Acid) ---
			if (type == WATER || type == ACID || type == ACIDIC_WATER) {
				if (y < SIM_HEIGHT - 1) { //! Boundary check: Don't fall off screen
					//! 1. Try to fall straight down
					//! We check 'grid' (is it currently empty?) AND 'nextGrid' (did someone else just move there?)
					if (grid[below] == EMPTY && nextGrid[below] == EMPTY) {
						nextGrid[below] = type; nextGrid[i] = EMPTY; moved = true;
					}
					//! 2. Try to flow diagonally down-left (only if left is empty too - prevents leaking walls)
					else if (scanX > 0 && grid[belowL] == EMPTY && nextGrid[belowL] == EMPTY && grid[left] == EMPTY) {
						nextGrid[belowL] = type; nextGrid[i] = EMPTY; moved = true;
					}
					//! 3. Try to flow diagonally down-right
					else if (scanX < SIM_WIDTH - 1 && grid[belowR] == EMPTY && nextGrid[belowR] == EMPTY && grid[right] == EMPTY) {
						nextGrid[belowR] = type; nextGrid[i] = EMPTY; moved = true;
					}
				}
				//! 4. If gravity failed, try to flow horizontally (Spread)
				if (!moved) {
					int dir = GetRandomValue(0, 1) == 0 ? -1 : 1; //! Random direction (-1 or 1)
					int side = i + dir;
					//! Check bounds and if the target is empty
					if (scanX + dir >= 0 && scanX + dir < SIM_WIDTH && grid[side] == EMPTY && nextGrid[side] == EMPTY) {
						nextGrid[side] = type; nextGrid[i] = EMPTY; moved = true;
					}
				}

				//? --- CHEMICAL REACTIONS ---
				if (!moved) { //! Only react if stationary (improves stability)
					int nbs[] = { below, left, right }; //! Check neighbors
					for (int n : nbs) {
						if (!IsValid(n)) continue;
						int target = grid[n];

						//! Reaction 1: Acid turns Water into Acidic Water
						//! Only "active" acid (ACID or ACIDIC_WATER) causes this. Pure water doesn't react with itself.
						if ((type == ACID || type == ACIDIC_WATER) && target == WATER) {
							nextGrid[n] = ACIDIC_WATER;
						}

						//! Reaction 2: Pure Acid dissolves solids (Wall, Sand, Wood)
						if (type == ACID && (target == SAND || target == WOOD || target == WALL)) {
							//! Walls dissolve slowly (1 in 50 chance), others quickly (1 in 10)
							int chance = (target == WALL) ? 50 : 10;
							if (GetRandomValue(0, chance) == 0) {
								nextGrid[n] = EMPTY; //! Destroy target
								nextGrid[i] = EMPTY; //! Use up acid
								break;
							}
						}
					}
				}
			}
			// --- LOGIC FOR SOLIDS (Sand) ---
			else if (type == SAND) {
				if (y < SIM_HEIGHT - 1) {
					//! 1. Fall down
					if (grid[below] == EMPTY && nextGrid[below] == EMPTY) {
						nextGrid[below] = SAND; nextGrid[i] = EMPTY;
					}
					//! 2. Sink in Water (Density check)
					//! If below is water, swap positions (Sand goes down, Water goes up)
					else if ((grid[below] == WATER || grid[below] == ACIDIC_WATER) && nextGrid[below] == grid[below]) {
						nextGrid[below] = SAND; nextGrid[i] = grid[below];
					}
					//! 3. Slide down slopes (formation of piles)
					else if (scanX > 0 && grid[belowL] == EMPTY && nextGrid[belowL] == EMPTY) {
						nextGrid[belowL] = SAND; nextGrid[i] = EMPTY;
					}
					else if (scanX < SIM_WIDTH - 1 && grid[belowR] == EMPTY && nextGrid[belowR] == EMPTY) {
						nextGrid[belowR] = SAND; nextGrid[i] = EMPTY;
					}
				}
			}
		}
	}

	// --- PASS 2: TOP-TO-DOWN SCAN (Gases) ---
	//! Gases rise, so we must scan from top to bottom to prevent teleporting to the top.
	for (int y = 0; y < SIM_HEIGHT; y++) {
		for (int x = 0; x < SIM_WIDTH; x++) {
			int scanX = (y % 2 == 0) ? x : (SIM_WIDTH - 1 - x);
			int i = y * SIM_WIDTH + scanX;
			int type = grid[i];

			if (type != FIRE && type != SMOKE) continue;

			//! GAS LOGIC: If gas reaches the very top (y=0), remove it.
			if (y == 0) {
				nextGrid[i] = EMPTY;
				continue;
			}

			int above = i - SIM_WIDTH;
			int aboveL = i - SIM_WIDTH - 1;
			int aboveR = i - SIM_WIDTH + 1;
			bool moved = false;

			if (y > 0) {
				//! 1. Rise up
				if (grid[above] == EMPTY && nextGrid[above] == EMPTY) {
					nextGrid[above] = type; nextGrid[i] = EMPTY; moved = true;
				}
				//! 2. Rise diagonally (if blocked)
				else if (scanX > 0 && grid[aboveL] == EMPTY && nextGrid[aboveL] == EMPTY) {
					nextGrid[aboveL] = type; nextGrid[i] = EMPTY; moved = true;
				}
				else if (scanX < SIM_WIDTH - 1 && grid[aboveR] == EMPTY && nextGrid[aboveR] == EMPTY) {
					nextGrid[aboveR] = type; nextGrid[i] = EMPTY; moved = true;
				}
			}

			//! 3. Ceiling Spread (Reverse Liquid Logic)
			//! If smoke hits a ceiling (didn't move up), try to spread sideways
			if (!moved && type == SMOKE) {
				int dir = GetRandomValue(0, 1) == 0 ? -1 : 1;
				int side = i + dir;
				if (scanX + dir >= 0 && scanX + dir < SIM_WIDTH && grid[side] == EMPTY && nextGrid[side] == EMPTY) {
					nextGrid[side] = type; nextGrid[i] = EMPTY; moved = true;
				}
			}

			//! FIRE LOGIC: Burn wood
			if (type == FIRE) {
				int nbs[] = { i - 1, i + 1, i + SIM_WIDTH, i - SIM_WIDTH };
				for (int n : nbs) if (IsValid(n) && grid[n] == WOOD && GetRandomValue(0, 10) < 2) nextGrid[n] = FIRE;

				//! Fire dies out randomly and turns into smoke or nothing
				if (GetRandomValue(0, 100) < 10) nextGrid[i] = (GetRandomValue(0, 2) == 0 ? SMOKE : EMPTY);
			}

			//! SMOKE LOGIC: Randomly disappear in open air (very low chance to allow accumulation)
			if (type == SMOKE && !moved) {
				if (GetRandomValue(0, 500) == 0) nextGrid[i] = EMPTY;
			}
		}
	}
	//! Finally, update the main grid with the calculated next state
	grid = nextGrid;
}

// --- UI RENDERING ---
void DrawUI() {
	//! Draw background panel
	DrawRectangle(0, SIM_HEIGHT_PIXELS, SCREEN_WIDTH, UI_HEIGHT, Color{ 30, 30, 30, 255 });

	int buttonW = 65;
	int startX = 10;
	int startY = SIM_HEIGHT_PIXELS + 20;

	//! Loop through all elements to create buttons
	for (int i = 0; i < (int)elements.size(); i++) {
		int x = startX + (buttonW + 5) * i;

		//! Highlight the selected tool with a white border
		if (elements[i].id == currentTool) DrawRectangleLines(x - 2, startY - 2, buttonW + 4, 44, WHITE);

		//! Draw button
		DrawRectangle(x, startY, buttonW, 40, elements[i].color);
		DrawText(elements[i].name.c_str(), x + 5, startY + 15, 10, (elements[i].id == EMPTY || elements[i].id == SAND) ? WHITE : BLACK);

		//! Check for mouse click on button
		Vector2 m = GetMousePosition();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m.x > x && m.x < x + buttonW && m.y > startY && m.y < startY + 40)
			currentTool = elements[i].id;
	}
}

// --- MAIN FUNCTION ---
int main() {
	//! Initialize the window
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DINO");
	SetTargetFPS(60); //! Limit to 60 Frames Per Second

	//! CPU-side image buffer (where we manipulate pixels)
	Image simImage = GenImageColor(SIM_WIDTH, SIM_HEIGHT, BLACK);
	//! GPU-side texture (what we draw to the screen)
	Texture2D texture = LoadTextureFromImage(simImage);

	DebugOverlay debugger;

	// --- GAME LOOP ---
	while (!WindowShouldClose()) {
		Vector2 m = GetMousePosition();

		//! DEBUG MODE TOGGLE
		if (IsKeyPressed(KEY_D)) {
			debugger.Toggle();
		}

		//! RESET SIMULATION (R) --- BURAYI EKLEDIK
		if (IsKeyPressed(KEY_R)) {
			// std::fill kullanarak dizinin basindan sonuna kadar temizliyoruz (Daha hizli)
			std::fill(grid.begin(), grid.end(), EMPTY);
			std::fill(nextGrid.begin(), nextGrid.end(), EMPTY);
		}

		//! Input: Change brush size
		brushSize += (int)GetMouseWheelMove();
		if (brushSize < 1) brushSize = 1;

		//! Input: Drawing (Left Click)
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && m.y < SIM_HEIGHT_PIXELS) {
			int mx = (int)(m.x / SCALE); int my = (int)(m.y / SCALE);

			for (int y = -brushSize; y <= brushSize; y++) {
				for (int x = -brushSize; x <= brushSize; x++) {
					int nx = mx + x; int ny = my + y;
					if (nx >= 0 && nx < SIM_WIDTH && ny >= 0 && ny < SIM_HEIGHT) {
						if (currentTool != WALL && currentTool != EMPTY && grid[ny * SIM_WIDTH + nx] == WALL) continue;
						grid[ny * SIM_WIDTH + nx] = currentTool;
					}
				}
			}
		}

		//! 1. Update Physics
		UpdateSimulation();

		//! 2. Render CPU Image
		for (int i = 0; i < (int)grid.size(); i++) {
			Color c = BLACK;
			if (grid[i] != EMPTY) {
				c = GetColor(grid[i]);
				if (grid[i] == ACIDIC_WATER) c = ColorBrightness(c, (float)GetRandomValue(-10, 10) / 100.0f);
				if (grid[i] == FIRE) c = (GetRandomValue(0, 2) == 0) ? ORANGE : RED;
				if (grid[i] == SAND) c = ColorBrightness(c, (float)GetRandomValue(-5, 5) / 100.0f);
				if (grid[i] == ACID) c = (GetRandomValue(0, 3) == 0) ? LIME : c;
			}
			ImageDrawPixel(&simImage, i % SIM_WIDTH, i / SIM_WIDTH, c);
		}

		//! 3. Upload to GPU
		UpdateTexture(texture, simImage.data);

		//! 4. Draw to Screen
		BeginDrawing();
		ClearBackground(Color{ 20, 20, 20, 255 });

		DrawTexturePro(texture,
			Rectangle{ 0, 0, (float)SIM_WIDTH, (float)SIM_HEIGHT },
			Rectangle{ 0, 0, (float)SCREEN_WIDTH, (float)SIM_HEIGHT_PIXELS },
			Vector2{ 0, 0 }, 0.0f, WHITE);

		if (m.y < SIM_HEIGHT_PIXELS)
			DrawRectangleLines((int)m.x - (brushSize * SCALE), (int)m.y - (brushSize * SCALE), (brushSize * 2 + 1) * SCALE, (brushSize * 2 + 1) * SCALE, WHITE);

		DrawUI();

		//! Debugger Draw
		int hoverX = (int)(m.x / SCALE);
		int hoverY = (int)(m.y / SCALE);
		int index = hoverY * SIM_WIDTH + hoverX;
		int cellType = IsValid(index) ? grid[index] : -1;

		debugger.Draw(SCREEN_WIDTH, SIM_HEIGHT_PIXELS, SCALE, (int)m.x, (int)m.y, index, cellType);

		// --- INSPECTOR UI ---
		if (m.y < SIM_HEIGHT_PIXELS && m.x < SCREEN_WIDTH && m.x >= 0 && m.y >= 0) {
			int hoverX = (int)(m.x / SCALE);
			int hoverY = (int)(m.y / SCALE);
			int index = hoverY * SIM_WIDTH + hoverX;
			if (index >= 0 && index < (int)grid.size()) {
				int type = grid[index];
				if (type != EMPTY) {
					std::string elementName = GetName(type);
					DrawText(TextFormat("Hover: %s", elementName.c_str()), 10, 10, 20, RAYWHITE);
				}
			}
		}


		EndDrawing();
	}

	UnloadTexture(texture);
	UnloadImage(simImage);
	CloseWindow();
	return 0;
}