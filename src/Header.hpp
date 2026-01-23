#ifndef HEADER_HPP
#define HEADER_HPP
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

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
};

class ListItem{
    private:
        ItemTypes type;
        std::string path;
        std::string fileName;
        std::filesystem::file_time_type lastOpened;
    
    public:
        ListItem(ItemTypes t, std::string p, std::filesystem::file_time_type l);
        ListItem(ItemTypes type, std::string path);
        void SetType(ItemTypes t);
        ItemTypes GetType();
        void SetPath(std::string p);
        std::string GetPath();
        void SetFileName(std::string fn);
        std::string GetFileName();
        void SetLastOpened(std::filesystem::file_time_type l);
        std::filesystem::file_time_type GetLastOpened();
        std::string ToString();

};

struct Context{
  std::string homedir;
  nlohmann::json data; 
  SortTypes sortType;
  std::string currentPath;
  std::vector<ListItem*> currentContent;
  std::vector<std::string> currentStringified;
  std::string exception;
  std::vector<std::string> qNavPaths;
  std::vector<std::string> qNavEntries;
  int activeModalIndex;
  bool anyModalActive;
};

void PathToItemList(std::string path, Context& context);
void SortItemList(Context& context);

namespace ProcessingFuncs{
    void StringifyContent(Context& context);
    void ParseQNavPathsToEntries(Context& context);
    std::string FsTimeToString(std::filesystem::file_time_type time);
    std::time_t FsTimeToTimeT(std::filesystem::file_time_type time);
    void ReloadItemList(Context& context);
}

namespace ElementLogic{
    void OnSelectedMenuOption(Context& context, int& selected);
    void OnSelectedSortOption(Context& context);
    void OnSelectedQNavButton(Context& context, int& qNavSelected);
    void OnSelectedQNavAddButton(Context& context);
    void OnSelectedNewFileButton(Context& context, std::string name);
    void OnSelectedNewDirectoryButton(Context& context, std::string name);
    void OnSelectedDeleteElementButton(Context& context, int& selected);
    void OnSelectedRenameElementButton(Context& context, int& selected, std::string& newName);
}
#endif 