#include "Header.hpp"

#include <iostream>
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
      ""
    );
  currentContent.push_back(item);
  for (const auto & entry : fs::directory_iterator(path)){
    ItemTypes tempType = entry.is_directory() ? ItemTypes::DIR : ItemTypes::FIL;
    //try to find time of last write
    std::string lwt = "";
    try{
      lwt = ProcessingFuncs::FsTimeToString(entry.last_write_time());
    }
    catch(fs::filesystem_error error){
      lwt = "N/A                     "; //buncha spaces so it lines up nicely with the rest of the files...
    }
    ListItem* item = new ListItem(
      tempType, 
      std::string(entry.path()), 
      lwt
    );
    currentContent.push_back(item);
  }
}

void SortItemList(std::vector<ListItem*>& currentContent, SortTypes sortType = SortTypes::NAME_ASC){
    
}