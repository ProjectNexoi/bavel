#ifndef HEADER_HPP
#define HEADER_HPP
#include <filesystem>
#include <string>
#include <vector>
#include "ftxui/component/screen_interactive.hpp"

// Declarations

enum ItemTypes{
    DIR = 0,
    FIL = 1,
    BACK = 2
};

enum SortTypes{
    NAME_ASC = 0,
    NAME_DESC = 1,
    TIME_ASC = 2,
    TIME_DESC = 3,
    TYPE_ASC = 4,
    TYPE_DESC = 5
};

class ListItem{
    private:
        ItemTypes type;
        std::string name;
        std::string lastOpened;
    
    public:
        ListItem(ItemTypes t, std::string n, std::string l);
        ListItem(ItemTypes type, std::string name);
        void SetType(ItemTypes t);
        ItemTypes GetType();
        void SetName(std::string n);
        std::string GetName();
        void SetLastOpened(std::string l);
        std::string GetLastOpened();
        std::string ToString();

};

void PathToItemList(std::string path, std::vector<ListItem*>& currentContent);
void SortItemList(std::vector<ListItem*>& currentContent, SortTypes sortType);

namespace ProcessingFuncs{
    void StringifyContent(std::vector<ListItem*>& currentContent, 
        std::vector<std::string>& currentStringified);
    void OnSelectedMenuOption(
        std::vector<ListItem*>& currentContent, 
        std::vector<std::string>& currentStringified,
        std::string& currentPath,
        int& selected,
        std::string& exception);
    std::string FsTimeToString(std::filesystem::file_time_type time);
}


#endif 