#include "Header.hpp"
#include <fstream>

namespace DataLoader{
    void LoadFileToData(Context& context){
        std::ifstream(std::string(context.homedir) + "/.bavel/data.json") >> context.data;
    }

    void SaveDataToFile(Context& context){
        std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
    }
}