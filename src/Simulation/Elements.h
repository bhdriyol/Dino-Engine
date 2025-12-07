#pragma once
#include "raylib.h"
#include <string>
#include <vector>

//! Enumeration of all Element IDs
enum ElementType {
    EMPTY = 0,
    WALL = 1,
    SAND = 2,
    WATER = 3,
    WOOD = 4,
    FIRE = 5,
    SMOKE = 6,
    ACID = 7,
    ACIDIC_WATER = 8,
    STEAM = 9,
    ICE = 10,
    LAVA = 11,
    STONE = 12,
    GLASS = 13,
    GUNPOWDER = 14,
    TOOL_HEAT = 98,
    TOOL_COOL = 99
};

//! Physical states of matter for generic physics logic
enum ElementState {
    STATE_STATIC = 0,  //! Walls, Wood, Ice (Immovable)
    STATE_POWDER = 1,  //! Sand, Gunpowder (Falls, piles up)
    STATE_LIQUID = 2,  //! Water, Acid, Lava (Flows horizontally)
    STATE_GAS = 3      //! Smoke, Steam (Rises)
};

//! Structure defining all properties of an element
struct ElementDef {
    int id;                 //! Unique ID
    std::string name;       //! Display name for UI
    Color color;            //! Base render color

    //! --- Physics Properties ---
    int state;              //! Physical state (Solid, Liquid, Gas)
    float baseTemp;         //! Default temperature in Celsius
    float heatConductivity; //! Heat transfer rate (0.0 = Insulator, 1.0 = Conductor)
    float coolingRate;      //! How fast it loses heat to ambient (0.0 = Retains heat well)

    //! --- Phase Change Properties ---
    float highTemp;         //! Temperature threshold for heating
    int highTempConvert;    //! Element ID to convert to when heated (-1 = No change)

    float lowTemp;          //! Temperature threshold for cooling
    int lowTempConvert;     //! Element ID to convert to when cooled (-1 = No change)

    float flammability;     //! Probability of catching fire (0.0 = Non-flammable)
};

//! Global access to the element registry
extern std::vector<ElementDef> elements;

//! Helper functions
Color GetElementColor(int id);
std::string GetElementName(int id);
const ElementDef& GetElementDef(int id);