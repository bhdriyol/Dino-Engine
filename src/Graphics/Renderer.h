#pragma once
#include "raylib.h"
#include "Simulation/World.h"

class Renderer {
private:
    Image simImage;
    Texture2D texture;
    int simWidth;
    int simHeight;

    //! Flag for Thermal Vision mode
    bool thermalMode = false;

public:
    Renderer(int w, int h);
    ~Renderer();

    //! Toggles heatmap rendering
    void ToggleThermalMode() { thermalMode = !thermalMode; }
    bool IsThermalMode() const { return thermalMode; }

    //! Renders the simulation grid to the texture
    void DrawSimulation(const World& world);

    //! Renders the UI toolbar
    void DrawUI(int& currentTool);
};