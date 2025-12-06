#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include "raylib.h"
#include <string>

//! Handles onscreen debug information and grid visualization
class DebugOverlay {
public:
    bool isActive = false;

    void Toggle() { isActive = !isActive; }

    void Draw(int screenW, int screenH, int scale, int mouseX, int mouseY, int gridIndex, int cellType, float temp) {
        if (!isActive) return;

        //! Draw grid lines
        for (int x = 0; x <= screenW; x += scale) DrawLine(x, 0, x, screenH, Fade(WHITE, 0.05f));
        for (int y = 0; y <= screenH; y += scale) DrawLine(0, y, screenW, y, Fade(WHITE, 0.05f));

        int cellX = mouseX / scale;
        int cellY = mouseY / scale;

        if (mouseY < screenH) {
            DrawRectangleLines(cellX * scale, cellY * scale, scale, scale, RED);

            //! Tooltip box
            int infoX = mouseX + 15;
            int infoY = mouseY + 15;

            DrawRectangle(infoX, infoY, 160, 85, Fade(BLACK, 0.8f));
            DrawRectangleLines(infoX, infoY, 160, 85, GREEN);

            DrawText(TextFormat("Grid X, Y: [%d, %d]", cellX, cellY), infoX + 5, infoY + 5, 10, GREEN);
            DrawText(TextFormat("Array Idx: %d", gridIndex), infoX + 5, infoY + 20, 10, GREEN);
            DrawText(TextFormat("Type ID: %d", cellType), infoX + 5, infoY + 35, 10, YELLOW);
            DrawText(TextFormat("FPS: %d", GetFPS()), infoX + 5, infoY + 50, 10, RED);

            //! Temperature readout
            DrawText(TextFormat("Temp: %.1f C", temp), infoX + 5, infoY + 65, 10, ORANGE);
        }
    }
};

#endif