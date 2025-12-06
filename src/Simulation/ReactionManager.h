#pragma once
#include "World.h"
#include "Elements.h"
#include "raylib.h"

namespace ReactionManager {

    //! --- PHASE CHANGE LOGIC ---
    //! Checks if the cell needs to change state based on temperature
    inline void ProcessTemperature(World& world, int index) {
        int type = world.GetCell(index);
        if (type == EMPTY || type == WALL) return;

        float temp = world.GetTemp(index);
        const ElementDef& def = GetElementDef(type);

        //! High Temperature Conversion (Melting / Boiling)
        if (def.highTempConvert != -1 && temp > def.highTemp) {
            //! Add randomness to avoid uniform transitions
            if (GetRandomValue(0, 10) == 0) {
                float currentTemp = world.GetTemp(index);

                world.SetCell(index, def.highTempConvert);

                world.SetTemp(index, currentTemp);
            }
        }
        //! Low Temperature Conversion (Freezing / Condensation)
        else if (def.lowTempConvert != -1 && temp < def.lowTemp) {
            if (GetRandomValue(0, 50) == 0) {
                world.SetCell(index, def.lowTempConvert);
            }
        }

        //! Flammability Check (Spontaneous Combustion)
        if (def.flammability > 0 && temp > 300.0f) {
            if (GetRandomValue(0, (int)(1000 * (1.0f - def.flammability))) == 0) {
                world.SetCell(index, FIRE);
                world.SetTemp(index, 800.0f + GetRandomValue(0, 200));
            }
        }
    }

    //! --- CHEMICAL INTERACTIONS ---
    //! Checks reactions between a cell and its neighbor
    inline bool Interact(World& world, int selfIndex, int neighborIndex) {
        if (!world.IsValid(neighborIndex)) return false;

        int selfType = world.GetCell(selfIndex);
        int neighborType = world.GetCell(neighborIndex);

        if (neighborType == EMPTY || neighborType == WALL) return false;

        //! ACID REACTIONS
        if (selfType == ACID || selfType == ACIDIC_WATER) {
            //! Acid contaminates Water
            if (neighborType == WATER) {
                world.SetCell(neighborIndex, ACIDIC_WATER);
                return true;
            }
            //! Acid dissolves solids
            if (selfType == ACID && (neighborType == SAND || neighborType == WOOD || neighborType == STONE)) {
                if (GetRandomValue(0, 20) == 0) {
                    world.SetCell(neighborIndex, SMOKE); //! Dissolve into smoke
                    world.SetCell(selfIndex, EMPTY);     //! Consume acid
                    return true;
                }
            }
        }

        
        //! STEAM CONDENSATION (Steam + Water/Ice = Water)
        if (selfType == STEAM && (neighborType == WATER || neighborType == ICE)) {
            if (GetRandomValue(0, 100) == 0) {
                world.SetCell(selfIndex, WATER);
                return true;
            }
        }
        

        
        //! LAVA COOLING (Lava + Water = Stone + Steam)
        if (selfType == LAVA && neighborType == WATER) {
            world.SetCell(selfIndex, STONE);     //! Lava becomes stone
            world.SetCell(neighborIndex, STEAM); //! Water evaporates
            return true;
        }
        

        return false;
    }
}