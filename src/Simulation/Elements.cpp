#include "Elements.h"

//! Registry of all elements
//! Format: { ID, NAME, COLOR, STATE, BASE_TEMP, CONDUCTIVITY, COOLING_RATE, HIGH_T, HIGH_CONV, LOW_T, LOW_CONV, FLAMMABILITY }
std::vector<ElementDef> elements = {
    //! 0: AIR
    { EMPTY, "AIR", BLACK, STATE_GAS, 22.0f, 0.4f, 0.01f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! 1: WALL
    { WALL, "WALL", DARKGRAY, STATE_STATIC, 22.0f, 0.05f, 0.0005f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! 2: SAND
    { SAND, "SAND", GOLD, STATE_POWDER, 22.0f, 1.0f, 0.0008f, 1700.0f, GLASS, -9999.0f, -1, 0.0f },

    //! 3: WATER
    { WATER, "WATER", SKYBLUE, STATE_LIQUID, 20.0f, 0.4f, 0.001f, 100.0f, STEAM, 0.0f, ICE, 0.0f },

    //! 4: WOOD
    { WOOD, "WOOD", BROWN, STATE_STATIC, 22.0f, 0.1f, 0.01f, 300.0f, FIRE, -9999.0f, -1, 0.4f },

    //! 5: FIRE
    { FIRE, "FIRE", ORANGE, STATE_GAS, 1200.0f, 0.8f, 0.0f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! 6: SMOKE
    { SMOKE, "SMOKE", {150,150,150,180}, STATE_GAS, 600.0f, 0.3f, 0.05f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! 7: ACID
    { ACID, "ACID", LIME, STATE_LIQUID, 20.0f, 0.4f, 0.02f, 120.0f, STEAM, -9999.0f, -1, 0.1f },

    //! 8: ACIDIC_WATER
    { ACIDIC_WATER, "A.WATER", {0,240,200,200}, STATE_LIQUID, 25.0f, 0.4f, 0.02f, 110.0f, STEAM, -9999.0f, -1, 0.0f },

    //! 9: STEAM
    { STEAM, "STEAM", RAYWHITE, STATE_GAS, 150.0f, 0.2f, 0.1f, 9999.0f, -1, 99.0f, WATER, 0.0f },

    //! 10: ICE
    { ICE, "ICE", {200, 200, 255, 255}, STATE_STATIC, -10.0f, 0.3f, 0.01f, 1.0f, WATER, -9999.0f, -1, 0.0f },

    //! 11: LAVA
    { LAVA, "LAVA", {255, 80, 0, 255}, STATE_LIQUID, 1200.0f, 0.5f, 0.005f, 9999.0f, -1, 700.0f, STONE, 0.0f },

    //! 12: STONE
    { STONE, "STONE", DARKGRAY, STATE_STATIC, 22.0f, 0.05f, 0.002f, 1100.0f, LAVA, -9999.0f, -1, 0.0f },

    //! 13: GLASS
    { GLASS, "GLASS", {200, 255, 255, 150}, STATE_STATIC, 22.0f, 1.0f, 0.005f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! 14: GUNPOWDER
    { GUNPOWDER, "GUNPOWDER", {50, 50, 50, 255}, STATE_POWDER, 22.0f, 0.2f, 0.01f, 250.0f, FIRE, -9999.0f, -1, 0.9f },

    //! HEAT TOOL
    { TOOL_HEAT, "HEAT", RED, STATE_STATIC, 0.0f, 0.0f, 0.0f, 9999.0f, -1, -9999.0f, -1, 0.0f },

    //! COOL TOOL
    { TOOL_COOL, "COOL", BLUE, STATE_STATIC, 0.0f, 0.0f, 0.0f, 9999.0f, -1, -9999.0f, -1, 0.0f }
};

Color GetElementColor(int id) {
    for (const auto& e : elements) if (e.id == id) return e.color;
    return BLACK;
}

std::string GetElementName(int id) {
    for (const auto& e : elements) if (e.id == id) return e.name;
    return "AIR";
}

const ElementDef& GetElementDef(int id) {
    for (const auto& e : elements) if (e.id == id) return e;
    return elements[0];
}