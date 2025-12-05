#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include "raylib.h"
#include <string>


class DebugOverlay {
public:
	bool isActive = false; //! Is debug mode currently enabled?

	//! Toggles the debug state
	void Toggle() {
		isActive = !isActive;
	}

	//! Main function to draw grid lines and info box
	void Draw(int screenW, int screenH, int scale, int mouseX, int mouseY, int gridIndex, int cellType) {
		if (!isActive) return; //! Do nothing if debug mode is off

		//! 1. Draw Grid Lines (Slightly transparent)
		//! Vertical lines
		for (int x = 0; x <= screenW; x += scale) {
			DrawLine(x, 0, x, screenH, Fade(WHITE, 0.05f));
		}
		//! Horizontal lines
		for (int y = 0; y <= screenH; y += scale) {
			DrawLine(0, y, screenW, y, Fade(WHITE, 0.05f));
		}

		//! 2. Highlight the specific cell under the mouse with a red border
		int cellX = mouseX / scale;
		int cellY = mouseY / scale;

		//! Only draw if within the simulation bounds
		if (mouseY < screenH) {
			DrawRectangleLines(cellX * scale, cellY * scale, scale, scale, RED);

			//! 3. Info Box (Floating tooltip near the mouse)
			int infoX = mouseX + 15;
			int infoY = mouseY + 15;

			//! Dark background for readability
			DrawRectangle(infoX, infoY, 160, 70, Fade(BLACK, 0.8f));
			DrawRectangleLines(infoX, infoY, 160, 70, GREEN);

			//! Grid Coordinates
			DrawText(TextFormat("Grid X, Y: [%d, %d]", cellX, cellY), infoX + 5, infoY + 5, 10, GREEN);

			//! Array Index (Position in the 1D vector)
			DrawText(TextFormat("Array Idx: %d", gridIndex), infoX + 5, infoY + 20, 10, GREEN);

			//! Cell Type ID (What element is here?)
			DrawText(TextFormat("Cell Type ID: %d", cellType), infoX + 5, infoY + 35, 10, YELLOW);

			//! Frames Per Second
			DrawText(TextFormat("FPS: %d", GetFPS()), infoX + 5, infoY + 50, 10, RED);
		}
	}
};

#endif