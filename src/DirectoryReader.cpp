#include "Header.hpp"

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
#include <system_error>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>
#include <chrono>
namespace fs = std::filesystem;

#include "ftxui/screen/terminal.hpp"

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"  
#include "ftxui/component/component_options.hpp"  
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/util/ref.hpp"
#include "ftxui/component/loop.hpp"

void PathToItemList(std::string path, Context& context){
    if (access(path.c_str(), R_OK | X_OK) != 0) {
        context.exception = "Permission denied";
        return;
    }
    for( auto& entry : context.currentContent){
        delete entry;
    }
    context.currentContent.clear();
    ListItem* item = new ListItem(ItemTypes::BACK, "..", fs::file_time_type::min());
    context.currentContent.push_back(item);
    for (const auto & entry : fs::directory_iterator(path)){
        ListItem* item = new ListItem(entry);
        context.currentContent.push_back(item);
    }
}

void SortItemList(Context& context){
    std::sort(context.currentContent.begin(), context.currentContent.end(), [&context](ListItem* a, ListItem* b) {
        if (a->GetType() == ItemTypes::BACK && b->GetType() != ItemTypes::BACK) return true;
        if (b->GetType() == ItemTypes::BACK && a->GetType() != ItemTypes::BACK) return false;
        if (a->GetType() == ItemTypes::BACK && b->GetType() == ItemTypes::BACK) return false;

        switch(context.sortType) {
            case SortTypes::NAME_ASC:
                return a->GetPath() < b->GetPath();
            case SortTypes::NAME_DESC:
                return a->GetPath() > b->GetPath();
            case SortTypes::TIME_ASC:
                return a->GetLastOpened() < b->GetLastOpened();
            case SortTypes::TIME_DESC:
                return a->GetLastOpened() > b->GetLastOpened();
            default:
                return a->GetPath() < b->GetPath();
        }
    });
}

void NavigateToPath(Context& context, std::string path){
    if (access(path.c_str(), R_OK | X_OK) != 0) {
        return;
    }
    PathToItemList(path, context);
    SortItemList(context);
    context.currentPath = path;
    ProcessingFuncs::StringifyContent(context);
    ProcessingFuncs::ParseCurrentPathToNavText(context);
}

uintmax_t GetDirectorySize(std::filesystem::directory_entry entry){
    uintmax_t size = 0;
    for (const auto & entry : fs::recursive_directory_iterator(entry.path(), fs::directory_options::skip_permission_denied)){
        if(entry.is_regular_file()){
            size += entry.file_size();
        }
    }
    return size;
}
    