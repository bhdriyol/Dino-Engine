#pragma once
#include <vector>

class World {
private:
    //! Double buffering for particle data
    std::vector<int> grid;
    std::vector<int> nextGrid;

    //! Double buffering for temperature data (Thermodynamics)
    std::vector<float> gridTemp;
    std::vector<float> nextGridTemp;

    int width;
    int height;

public:
    //! Constructor: Initializes grids
    World(int w, int h);

    //! Main update loop: Physics + Thermodynamics
    void Update();

    //! Clears the world and resets temperature
    void Reset();

    //! Boundary check
    bool IsValid(int index) const;

    //! Setters and Getters
    void SetCell(int index, int type);
    int GetCell(int index) const;

    //! Temperature Access
    float GetTemp(int index) const;
    void SetTemp(int index, float temp);

    //! Data access for Renderer (Const references for performance)
    const std::vector<int>& GetGridData() const { return grid; }
    const std::vector<float>& GetTempData() const { return gridTemp; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
};