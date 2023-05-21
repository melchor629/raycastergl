#include <arguments.hpp>
#include <argumentum/argparse.h>
#include <argumentum/argparse-h.h>

using namespace argumentum;
using namespace glm;

bool Arguments::parseArguments(int argc, const char* const argv[]) {
    argument_parser parser;
    auto params = parser.params();

    parser.config()
        .program(argv[0])
        .description("Wolf3D-like raycaster");

    params.add_parameter(vsync, "--vsync")
        .nargs(1)
        .absent(1)
        .help("Enables or disables V-Sync. 0 or -1 to disable, 1 to enable, 2 or higher to enable and reduce FPS (defaults to 1)");
    params.add_parameter(map, "--map")
        .nargs(1)
        .absent("default.yaml")
        .help("Uses a diferent map yaml, found in the maps folder inside resources (defaults to default.yaml)");
    params.add_parameter(initialWindowSize, "--window-size", "-s")
        .nargs(1)
        .absent({ 1333, 1000 })
        .action([] (auto& vec, const std::string& value, Environment& env) {
            size_t xPos = value.find("x");
            size_t commaPos = value.find(",");
            if(xPos == std::string::npos && commaPos == std::string::npos) {
                env.add_error("Window size format is invalid: " + value);
                return;
            }

            auto p = xPos != std::string::npos ? xPos : commaPos;
            auto widthString = value.substr(0, p);
            auto heightString = value.substr(p + 1);
            vec.x = std::stoi(widthString);
            vec.y = std::stoi(heightString);

            if(vec.x < 133) {
                env.add_error("Window width is invalid (minimum 133): " + widthString);
            }

            if(vec.y < 100) {
                env.add_error("Window height is invalid (minimum 100): " + widthString);
            }
        })
        .help("Specifies the initial window size (defaults to 1333x1000)");

    return parser.parse_args(argc, (char**) (void*) argv, 1);
}
