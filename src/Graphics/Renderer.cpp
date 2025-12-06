#include "Renderer.h"
#include "Core/Constants.h"
#include "Simulation/Elements.h"

Renderer::Renderer(int w, int h) : simWidth(w), simHeight(h) {
    simImage = GenImageColor(w, h, BLACK);
    texture = LoadTextureFromImage(simImage);
}

Renderer::~Renderer() {
    UnloadTexture(texture);
    UnloadImage(simImage);
}

//! Helper: Linear Interpolation for Colors
static Color ColorLerp(Color c1, Color c2, float amount) {
    if (amount < 0) amount = 0;
    if (amount > 1) amount = 1;
    return Color{
        (unsigned char)(c1.r + amount * (c2.r - c1.r)),
        (unsigned char)(c1.g + amount * (c2.g - c1.g)),
        (unsigned char)(c1.b + amount * (c2.b - c1.b)),
        255
    };
}

void Renderer::DrawSimulation(const World& world) {
    const std::vector<int>& grid = world.GetGridData();
    const std::vector<float>& gridTemp = world.GetTempData();

    for (int i = 0; i < (int)grid.size(); i++) {
        Color c = BLACK;

        if (thermalMode) {
            //! --- THERMAL MODE RENDER ---
            float temp = gridTemp[i];

            // Color Palette
            Color c_cold = { 0, 0, 150, 255 };
            Color c_ambient = { 0, 120, 255, 255 };
            Color c_room = { 0, 255, 255, 255 };
            Color c_warm = { 0, 255, 0, 255 };
            Color c_hot = { 255, 255, 0, 255 };
            Color c_fire = { 255, 0, 0, 255 };
            Color c_plasma = { 255, 255, 255, 255 };


            if (temp < 0.0f) {
                c = c_cold;
            }
            else if (temp < 25.0f) {
                float t = (temp - 0.0f) / 25.0f;
                c = ColorLerp(c_ambient, c_room, t);
            }
            else if (temp < 100.0f) {
                float t = (temp - 25.0f) / 75.0f;
                c = ColorLerp(c_room, c_warm, t);
            }
            else if (temp < 400.0f) {
                float t = (temp - 100.0f) / 300.0f;
                c = ColorLerp(c_warm, c_hot, t);
            }
            else if (temp < 1000.0f) {
                float t = (temp - 400.0f) / 600.0f;
                c = ColorLerp(c_hot, c_fire, t);
            }
            else {
                float t = (temp - 1000.0f) / 1000.0f;
                if (t > 1.0f) t = 1.0f;
                c = ColorLerp(c_fire, c_plasma, t);
            }
        }
        else {
            //! --- STANDARD RENDER ---
            if (grid[i] != EMPTY) {
                c = GetElementColor(grid[i]);
                //! Add noise for liquid/gas visuals
                if (grid[i] == ACIDIC_WATER) c = ColorLerp(c, WHITE, GetRandomValue(-10, 10) / 100.0f);
                if (grid[i] == FIRE) c = (GetRandomValue(0, 2) == 0) ? ORANGE : RED;
                if (grid[i] == SAND) c = ColorLerp(c, BLACK, GetRandomValue(0, 5) / 100.0f);
                if (grid[i] == ACID) c = (GetRandomValue(0, 3) == 0) ? LIME : c;
            }
        }
        ImageDrawPixel(&simImage, i % simWidth, i / simWidth, c);
    }

    UpdateTexture(texture, simImage.data);

    DrawTexturePro(texture,
        Rectangle{ 0, 0, (float)simWidth, (float)simHeight },
        Rectangle{ 0, 0, (float)Config::SCREEN_WIDTH, (float)Config::SIM_HEIGHT_PIXELS },
        Vector2{ 0, 0 }, 0.0f, WHITE);
}

void Renderer::DrawUI(int& currentTool) {
    DrawRectangle(0, Config::SIM_HEIGHT_PIXELS, Config::SCREEN_WIDTH, Config::UI_HEIGHT, Color{ 30, 30, 30, 255 });

    int buttonW = 65;
    int startX = 10;
    int startY = Config::SIM_HEIGHT_PIXELS + 20;

    for (int i = 0; i < (int)elements.size(); i++) {
        int x = startX + (buttonW + 5) * i;
        if (elements[i].id == currentTool) DrawRectangleLines(x - 2, startY - 2, buttonW + 4, 44, WHITE);
        DrawRectangle(x, startY, buttonW, 40, elements[i].color);
        DrawText(elements[i].name.c_str(), x + 5, startY + 15, 10, (elements[i].id == EMPTY || elements[i].id == SAND) ? WHITE : BLACK);

        Vector2 m = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m.x > x && m.x < x + buttonW && m.y > startY && m.y < startY + 40)
            currentTool = elements[i].id;
    }
}


//TODO Advanced Thermal Mode
/*
void Renderer::DrawSimulation(const World& world) {
    const std::vector<int>& grid = world.GetGridData();
    const std::vector<float>& gridTemp = world.GetTempData();

    for (int i = 0; i < (int)grid.size(); i++) {
        Color c = BLACK;

        if (thermalMode) {
            //! --- THERMAL MODE RENDER ---
            float temp = gridTemp[i];

            // Color Palette
            Color c_cold = { 0, 0, 150, 255 };     
            Color c_ambient = { 0, 120, 255, 255 };   
            Color c_room = { 0, 255, 255, 255 };   
            Color c_warm = { 0, 255, 0, 255 };     
            Color c_hot = { 255, 255, 0, 255 };   
            Color c_fire = { 255, 0, 0, 255 };    
            Color c_plasma = { 255, 255, 255, 255 };

            
            if (temp < 0.0f) {
                c = c_cold;
            }
            else if (temp < 25.0f) {
                float t = (temp - 0.0f) / 25.0f;
                c = ColorLerp(c_ambient, c_room, t);
            }
            else if (temp < 100.0f) {
                float t = (temp - 25.0f) / 75.0f;
                c = ColorLerp(c_room, c_warm, t);
            }
            else if (temp < 400.0f) {
                float t = (temp - 100.0f) / 300.0f;
                c = ColorLerp(c_warm, c_hot, t);
            }
            else if (temp < 1000.0f) {
                float t = (temp - 400.0f) / 600.0f;
                c = ColorLerp(c_hot, c_fire, t);
            }
            else {         
                float t = (temp - 1000.0f) / 1000.0f;
                if (t > 1.0f) t = 1.0f;
                c = ColorLerp(c_fire, c_plasma, t);
            }
        }
        else {
            //! --- STANDARD RENDER ---
            if (grid[i] != EMPTY) {
                c = GetElementColor(grid[i]);
                //! Add noise for liquid/gas visuals
                if (grid[i] == ACIDIC_WATER) c = ColorLerp(c, WHITE, GetRandomValue(-10, 10) / 100.0f);
                if (grid[i] == FIRE) c = (GetRandomValue(0, 2) == 0) ? ORANGE : RED;
                if (grid[i] == SAND) c = ColorLerp(c, BLACK, GetRandomValue(0, 5) / 100.0f);
                if (grid[i] == ACID) c = (GetRandomValue(0, 3) == 0) ? LIME : c;
            }
        }
        ImageDrawPixel(&simImage, i % simWidth, i / simWidth, c);
    }
*/