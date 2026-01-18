#include "Header.hpp"

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
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

void PathToItemList(std::string path, std::vector<ListItem*>& currentContent){
  currentContent.clear();
  ListItem* item = new ListItem(
      ItemTypes::BACK, 
      "..", 
      fs::file_time_type::min()
    );
  currentContent.push_back(item);
  for (const auto & entry : fs::directory_iterator(path)){
    ItemTypes tempType = entry.is_directory() ? ItemTypes::DIR : ItemTypes::FIL;
    //try to find time of last write
    fs::file_time_type lwt = fs::file_time_type::min();
    try{
      lwt = entry.last_write_time();
    }
    catch(fs::filesystem_error error){ lwt = fs::file_time_type::min(); }
    ListItem* item = new ListItem(
      tempType, 
      std::string(entry.path()), 
      lwt
    );
    currentContent.push_back(item);
  }
}

void SortItemList(std::vector<ListItem*>& currentContent, SortTypes sortType){
    std::sort(currentContent.begin(), currentContent.end(), [sortType](ListItem* a, ListItem* b) {
        if (a->GetType() == ItemTypes::BACK && b->GetType() != ItemTypes::BACK) return true;
        if (b->GetType() == ItemTypes::BACK && a->GetType() != ItemTypes::BACK) return false;
        if (a->GetType() == ItemTypes::BACK && b->GetType() == ItemTypes::BACK) return false;

        switch(sortType) {
            case SortTypes::NAME_ASC:
                return a->GetName() < b->GetName();
            case SortTypes::NAME_DESC:
                return a->GetName() > b->GetName();
            case SortTypes::TIME_ASC:
                return a->GetLastOpened() < b->GetLastOpened();
            case SortTypes::TIME_DESC:
                return a->GetLastOpened() > b->GetLastOpened();
            default:
                return a->GetName() < b->GetName();
        }
    });
}