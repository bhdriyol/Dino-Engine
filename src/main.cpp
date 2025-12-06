#include "raylib.h"
#include "Core/Constants.h"
#include "Simulation/World.h"
#include "Simulation/Elements.h"
#include "Graphics/Renderer.h"
#include "Graphics/DebugOverlay.h"

int main() {
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "Dino");
    SetTargetFPS(60);

    //! Initialize Modules
    World world(Config::SIM_WIDTH, Config::SIM_HEIGHT);
    Renderer renderer(Config::SIM_WIDTH, Config::SIM_HEIGHT);
    DebugOverlay debugger;

    int currentTool = SAND;
    int brushSize = 3;

    while (!WindowShouldClose()) {
        Vector2 m = GetMousePosition();

        //! --- INPUT HANDLING ---
        if (IsKeyPressed(KEY_D)) debugger.Toggle();
        if (IsKeyPressed(KEY_R)) world.Reset();
        if (IsKeyPressed(KEY_T)) renderer.ToggleThermalMode();

        brushSize += (int)GetMouseWheelMove();
        if (brushSize < 1) brushSize = 1;

        //! Drawing Input
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && m.y < Config::SIM_HEIGHT_PIXELS) {
            int mx = (int)(m.x / Config::SCALE);
            int my = (int)(m.y / Config::SCALE);

            for (int y = -brushSize; y <= brushSize; y++) {
                for (int x = -brushSize; x <= brushSize; x++) {
                    int nx = mx + x;
                    int ny = my + y;
                    int index = ny * Config::SIM_WIDTH + nx;

                    if (world.IsValid(index)) {
                        int cellType = world.GetCell(index);
                        const ElementDef& def = GetElementDef(cellType);
                        float currentT = world.GetTemp(index);

                        //! --- TOOL LOGIC ---
                        if (currentTool == TOOL_HEAT || currentTool == TOOL_COOL) {

                            //! 1. SOLID FILTER
                            if (cellType == EMPTY) continue;

                            //! 2. THERMAL RESISTANCE
                            float thermalResistance = 1.0f;
                            if (def.state == STATE_POWDER) thermalResistance = 5.0f;
                            else if (def.state == STATE_LIQUID) thermalResistance = 5.0f;
                            else if (def.state == STATE_STATIC) thermalResistance = 10.0f;

                            float changeAmount = 100.0f / thermalResistance;

                            if (currentTool == TOOL_HEAT) {
                                float newTemp = currentT + changeAmount;
                                if (newTemp > 9000.0f) newTemp = 9000.0f;
                                world.SetTemp(index, newTemp);
                            }
                            else if (currentTool == TOOL_COOL) {
                                float newTemp = currentT - changeAmount;
                                if (newTemp < -273.0f) newTemp = -273.0f;
                                world.SetTemp(index, newTemp);
                            }
                        }
                        //! --- NORMAL DRAW ---
                        else {
                            if (currentTool != WALL && currentTool != EMPTY && cellType == WALL) continue;
                            world.SetCell(index, currentTool);
                        }
                    }
                }
            }
        }

        //! --- UPDATE SIMULATION ---
        world.Update();

        //! --- DRAW FRAME ---
        BeginDrawing();
        ClearBackground(Color{ 20, 20, 20, 255 });

        renderer.DrawSimulation(world);
        renderer.DrawUI(currentTool);

        //! Cursor
        if (m.y < Config::SIM_HEIGHT_PIXELS)
            DrawRectangleLines((int)m.x - (brushSize * Config::SCALE), (int)m.y - (brushSize * Config::SCALE),
                (brushSize * 2 + 1) * Config::SCALE, (brushSize * 2 + 1) * Config::SCALE, WHITE);

        //! Debug & Info Overlay
        int hoverX = (int)(m.x / Config::SCALE);
        int hoverY = (int)(m.y / Config::SCALE);
        int index = hoverY * Config::SIM_WIDTH + hoverX;
        int cellType = world.GetCell(index);
        float cellTemp = world.GetTemp(index);

        debugger.Draw(Config::SCREEN_WIDTH, Config::SIM_HEIGHT_PIXELS, Config::SCALE, (int)m.x, (int)m.y, index, cellType, cellTemp);

        if (cellType != EMPTY && m.y < Config::SIM_HEIGHT_PIXELS) {
            DrawText(TextFormat(GetElementName(cellType).c_str()), 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("Temp: %.1f C", cellTemp), 10, 30, 20, RAYWHITE);
        }

        if (renderer.IsThermalMode()) DrawText("THERMAL MODE ON", 10, 30, 20, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}