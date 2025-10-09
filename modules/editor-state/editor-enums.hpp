#pragma once

#include <string>

namespace state {
enum class FLAG_EXPORT_OPTIONS {
    TGA, PNG
};
enum class FILL_MODE {
    PROVINCE, OWNER_AND_CONTROLLER
};
enum class CONTROL_MODE {
    NONE, SELECT, PICKING_COLOR, PAINTING, FILL, FILL_UNSAFE
};

inline std::string to_string(FILL_MODE MODE) {
        switch (MODE) {
                case FILL_MODE::PROVINCE:
                return "Province";
                case FILL_MODE::OWNER_AND_CONTROLLER:
                return "Owner&Control";
        }
}

enum class MAP_MODE {
    OWNER, CONTINENT, CULTURE, POP_DENSITY, RGO
};

inline std::string to_string(MAP_MODE MODE) {
        switch (MODE) {
            case MAP_MODE::OWNER:
            return "Political";
            case MAP_MODE::CULTURE:
            return "Culture population";
            case MAP_MODE::CONTINENT:
            return "Continents";
            case MAP_MODE::POP_DENSITY:
            return "Population density";
            case MAP_MODE::RGO:
            return "RGO";
        }
}

}