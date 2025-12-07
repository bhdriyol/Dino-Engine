#include "World.h"
#include "Elements.h"
#include "ReactionManager.h"
#include "raylib.h" 
#include <algorithm> 

const float AMBIENT_TEMP = 22.0f;

World::World(int w, int h) : width(w), height(h) {
    grid.resize(w * h, EMPTY);
    nextGrid.resize(w * h, EMPTY);
    gridTemp.resize(w * h, AMBIENT_TEMP);
    nextGridTemp.resize(w * h, AMBIENT_TEMP);
}

bool World::IsValid(int index) const {
    return index >= 0 && index < (int)grid.size();
}

void World::SetCell(int index, int type) {
    if (IsValid(index)) {
        nextGrid[index] = type;
        grid[index] = type;

        const ElementDef& def = GetElementDef(type);

        gridTemp[index] = def.baseTemp;
        nextGridTemp[index] = def.baseTemp;
    }
}

int World::GetCell(int index) const {
    if (IsValid(index)) return grid[index];
    return EMPTY;
}

float World::GetTemp(int index) const { if (IsValid(index)) return gridTemp[index]; return AMBIENT_TEMP; }

void World::SetTemp(int index, float temp) { if (IsValid(index)) { gridTemp[index] = temp; nextGridTemp[index] = temp; } }

void World::Reset() {
    std::fill(grid.begin(), grid.end(), EMPTY);
    std::fill(nextGrid.begin(), nextGrid.end(), EMPTY);
    std::fill(gridTemp.begin(), gridTemp.end(), AMBIENT_TEMP);
    std::fill(nextGridTemp.begin(), nextGridTemp.end(), AMBIENT_TEMP);
}

void World::Update() {
    nextGrid = grid;
    nextGridTemp = gridTemp;

    //! --- PHASE 1: THERMODYNAMICS & PHASE CHANGE ---
    for (int i = 0; i < (int)grid.size(); i++) {
        int type = grid[i];
        float myTemp = gridTemp[i];

        //! Get element properties safely
        const ElementDef& myDef = GetElementDef(type);

        //! 1. Heat Sources
        if (type == FIRE) {
            nextGridTemp[i] = 2200.0f + GetRandomValue(0, 300);
            myTemp = nextGridTemp[i];
        }

        //! 2. Heat Diffusion
        int nbs[] = { i - 1, i + 1, i - width, i + width };

        for (int n : nbs) {
            if (IsValid(n)) {
                float nTemp = gridTemp[n];

                //! Only take action if the neighbor is warmer than me 
                if (nTemp > myTemp) {
                    const ElementDef& nDef = GetElementDef(grid[n]);
                    float diff = nTemp - myTemp;

                    //! --- CONDUCTIVITY AND RATIO CALCULATION ---
                    float conductivity;
                    float rate = 0.05f; //! Default slow speed

                    bool amIGas = (myDef.state == STATE_GAS);
                    bool neighborGas = (nDef.state == STATE_GAS);

                    //! Static (Wall) control
                    bool amIStatic = (myDef.state == STATE_STATIC);
                    bool neighborStatic = (nDef.state == STATE_STATIC);

                    //! SCENARIO A: We are both Gas (Air-Fire) -> Very Fast
                    if (amIGas && neighborGas) {
                        conductivity = std::max(myDef.heatConductivity, nDef.heatConductivity);
                        rate = 0.25f; //! Gases mix very quickly
                    }
                    //! SCENARIO B: One of Us is Gas, the Other is "Moving Solid/Liquid" (Air-Sand) -> Fast
                    else if ((amIGas && !neighborStatic) || (neighborGas && !amIStatic)) {
                        conductivity = std::max(myDef.heatConductivity, nDef.heatConductivity);
                        rate = 0.15f;
                    }
                    //! SCENARIO C: Wall Involved -> Slow (Insulation)
                    else {
                        conductivity = (myDef.heatConductivity + nDef.heatConductivity) * 0.5f;
                        rate = 0.05f;
                    }

                    //! Radiation Bonus (Only if Not a Wall)
                    if (!amIStatic && !neighborStatic && diff > 800.0f) {
                        rate += 0.1f; //! Extra speed
                    }

                    //! Calculate Transfer
                    float transfer = diff * conductivity * rate;

                    //! Apply Heat 
                    nextGridTemp[i] += transfer;

                    //! Conservation of energy (Fire does not burn out, others cool down)
                    if (grid[n] != FIRE) {
                        nextGridTemp[n] -= transfer * 0.5f;
                    }
                }
            }
        }

        //! 3. Cooling
        float environmentFactor = 1.0f;
        if (myDef.state != STATE_GAS) environmentFactor = 0.5f;

        float cooling = (nextGridTemp[i] - AMBIENT_TEMP) * myDef.coolingRate * environmentFactor;

        nextGridTemp[i] -= cooling;

        //! 4. Phase Changes
        if (type != EMPTY && type != WALL) {
            ReactionManager::ProcessTemperature(*this, i);
        }

        //! --- (SAFETY CLAMP) ---
        if (nextGridTemp[i] > 5000.0f) nextGridTemp[i] = 5000.0f;
        //! Abosulte Zero
        if (nextGridTemp[i] < -273.0f) nextGridTemp[i] = -273.0f;
    }

    //! --- PHASE 2: GENERAL PARTICLE PHYSICS ---
    //! Iterating Bottom-Up for Solids and Liquids
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            int scanX = (y % 2 == 0) ? x : (width - 1 - x);
            int i = y * width + scanX;
            int type = grid[i];

            if (type == EMPTY || type == WALL) continue;

            const ElementDef& def = GetElementDef(type);
            int state = def.state;

            //! Skip statics and gases (handled elsewhere)
            if (state == STATE_STATIC) continue;
            if (state == STATE_GAS) continue;

            //! Calculate neighbor indices
            int below = i + width;
            int belowL = i + width - 1;
            int belowR = i + width + 1;

            int target = -1;

            if (y < height - 1) {
                //! 1. Gravity (Fall down)
                if (grid[below] == EMPTY && nextGrid[below] == EMPTY) target = below;

                //! 2. Density Check (Sinking in liquids)
                else if (state == STATE_POWDER) {
                    int belowType = grid[below];
                    if (GetElementDef(belowType).state == STATE_LIQUID && nextGrid[below] == belowType) {
                        target = below;
                        //! Swap particle and liquid
                        nextGrid[below] = type; nextGrid[i] = belowType;
                        //! Swap temperature
                        float t = nextGridTemp[below]; nextGridTemp[below] = nextGridTemp[i]; nextGridTemp[i] = t;
                        continue; //! Move handled, skip to next
                    }
                }

                //! 3. Dispersion (Slide down slopes)
                if (target == -1) {
                    if (scanX > 0 && grid[belowL] == EMPTY && nextGrid[belowL] == EMPTY) target = belowL;
                    else if (scanX < width - 1 && grid[belowR] == EMPTY && nextGrid[belowR] == EMPTY) target = belowR;
                }
            }

            //! 4. Horizontal Flow (Liquids only)
            if (state == STATE_LIQUID && target == -1) {
                int dir = GetRandomValue(0, 1) == 0 ? -1 : 1;
                int side = i + dir;
                if (scanX + dir >= 0 && scanX + dir < width && grid[side] == EMPTY && nextGrid[side] == EMPTY) target = side;

                //! Interaction with neighbors (Acid/Water/Lava mixing)
                int nbs[] = { below, i - 1, i + 1, i - width };
                for (int n : nbs) ReactionManager::Interact(*this, i, n);
            }

            //! Apply Movement
            if (target != -1) {
                nextGrid[target] = type;
                nextGrid[i] = EMPTY;
                //! Move heat with the particle
                nextGridTemp[target] = nextGridTemp[i];
                nextGridTemp[i] = AMBIENT_TEMP;
            }
        }
    }

    //! --- PHASE 3: GAS PHYSICS (Top-Down) ---
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int scanX = (y % 2 == 0) ? x : (width - 1 - x);
            int i = y * width + scanX;
            int type = grid[i];

            if (type == EMPTY || GetElementDef(type).state != STATE_GAS) continue;

            if (y == 0) { nextGrid[i] = EMPTY; continue; } //! Escape at ceiling

            int above = i - width;
            int aboveL = i - width - 1;
            int aboveR = i - width + 1;
            int target = -1;

            if (y > 0) {
                if (grid[above] == EMPTY && nextGrid[above] == EMPTY) target = above;
                else if (scanX > 0 && grid[aboveL] == EMPTY && nextGrid[aboveL] == EMPTY) target = aboveL;
                else if (scanX < width - 1 && grid[aboveR] == EMPTY && nextGrid[aboveR] == EMPTY) target = aboveR;
            }

            //! Ceiling spread behavior
            if (target == -1) {
                int dir = GetRandomValue(0, 1) == 0 ? -1 : 1;
                int side = i + dir;
                if (scanX + dir >= 0 && scanX + dir < width && grid[side] == EMPTY && nextGrid[side] == EMPTY) target = side;
            }

            //! Fire specific behavior (Burning wood)
            if (type == FIRE) {
                int fireNbs[] = { i - 1, i + 1, i - width, i + width };
                for (int n : fireNbs) if (IsValid(n) && grid[n] == WOOD && GetRandomValue(0, 20) == 0) {
                    nextGrid[n] = FIRE; nextGridTemp[n] = 1200.0f;
                }

                //? Need Smoke or not?
                ////! Fire dies out
                //if (GetRandomValue(0, 100) < 2) {
                //    nextGrid[i] = SMOKE;
                //    target = -1;
                //}
            }
            //! Smoke decay
            if (type == SMOKE && GetRandomValue(0, 1000) == 0) {
                nextGrid[i] = EMPTY;
                target = -1;
            }

            //! Apply Movement
            if (target != -1) {
                nextGrid[target] = type;
                nextGrid[i] = EMPTY;
                nextGridTemp[target] = nextGridTemp[i];
                nextGridTemp[i] = AMBIENT_TEMP;
            }
        }
    }

    grid = nextGrid;
    gridTemp = nextGridTemp;
}