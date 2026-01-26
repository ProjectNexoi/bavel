#ifndef HEADER_HPP
#define HEADER_HPP
#include <filesystem>
#include <future>
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

enum ReorderDirection{
    UP = 0,
    DOWN = 1
};

class ListItem{
    private:
        ItemTypes type;
        std::string path;
        std::string fileName;
        std::filesystem::file_time_type lastOpened;
        uintmax_t size;
        std::string owner;
        bool wasSizeFetched = false;
        bool wasOwnerFetched = false;
    
    public:
        ListItem(ItemTypes t, std::string p, std::filesystem::file_time_type l);
        ListItem(ItemTypes type, std::string path);
        ListItem(std::filesystem::directory_entry entry);
        void SetType(ItemTypes t);
        ItemTypes GetType();
        void SetPath(std::string p);
        std::string GetPath();
        void SetFileName(std::string fn);
        std::string GetFileName();
        void SetLastOpened(std::filesystem::file_time_type l);
        std::filesystem::file_time_type GetLastOpened();
        void SetSize(uintmax_t s);
        uintmax_t GetSize();
        void SetOwner(std::string o);
        std::string GetOwner();
        std::string ToString();

};


struct MetadataContext{
  std::string itemName = "";
  std::string itemType = "";
  std::string itemLastWrite = "";
  std::shared_future<uintmax_t> itemSize;
  std::string itemOwner = "";
  std::string itemPath = "";
};

struct Context{
  std::string homedir = "";
  nlohmann::json data = {};
  SortTypes sortType = SortTypes::NAME_ASC;
  std::string currentPath = "";
  std::vector<ListItem*> currentContent = {};
  std::vector<std::string> currentStringified = {};
  std::string exception = "";
  std::vector<std::string> qNavPaths = {};
  std::vector<std::string> qNavEntries = {};
  int activeModalIndex = 0;
  bool anyModalActive = false;
  std::string locationBarText = "";
  MetadataContext metadataContext;
};

void PathToItemList(std::string path, Context& context);
void SortItemList(Context& context);
void NavigateToPath(Context& context, std::string path);
uintmax_t GetDirectorySize(std::filesystem::directory_entry entry);

namespace ProcessingFuncs{
    void StringifyContent(Context& context);
    void ParseQNavPathsToEntries(Context& context);
    std::string FsTimeToString(std::filesystem::file_time_type time);
    std::time_t FsTimeToTimeT(std::filesystem::file_time_type time);
    void ReloadItemList(Context& context);
    void ParseCurrentPathToNavText(Context& context);
    std::string RawByteAmountToString(uintmax_t bytes);
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
    void OnLocationBarSubmit(Context& context);
    void UpdateInformationBox(Context& context, int& selected);
    void OnQNavReorder(Context& context, int& selected, ReorderDirection direction);
}

namespace DataLoader{
    void LoadFileToData(Context& context);
    void SaveDataToFile(Context& context);
}
#endif 