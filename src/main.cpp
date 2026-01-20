#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
namespace fs = std::filesystem;

#include "Header.hpp"
#include <nlohmann/json.hpp>

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

int main(){

  Context context;

  //Sets window to fill terminal
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  auto screen = ftxui::ScreenInteractive::FixedSize(size.ws_col,size.ws_row);

  //Get user's home directory
  struct passwd *pw = getpwuid(getuid());
  context.homedir = pw->pw_dir;

  //fetch json data
  if(!fs::exists(std::string(context.homedir) + "/.bavel/data.json")){ //if no data, then make the file
    fs::create_directories(std::string(context.homedir) + "/.bavel");
    std::ofstream(std::string(context.homedir) + "/.bavel/data.json");
  }
  if(!fs::is_empty(std::string(context.homedir) + "/.bavel/data.json")){
    std::ifstream(std::string(context.homedir) + "/.bavel/data.json") >> context.data;
  }

  context.sortType = SortTypes::NAME_ASC;
  
  //Fetches the current directory's content
  context.currentPath = "/";
  PathToItemList(context.currentPath, context);
  SortItemList(context);

  ProcessingFuncs::StringifyContent(context);
  std::string exception = "";

  //QuickNav entries
  std::vector<std::string> qNavPaths = {};
  try{
    qNavPaths = context.data["qNavEntries"].get<std::vector<std::string>>();
    
  }
  catch(std::exception& e){
    exception = e.what();
  }
  int qNavSelected = 0;
  auto qNavMenuOption = ftxui::MenuOption(ftxui::MenuOption::Vertical());
  qNavMenuOption.elements_prefix = []{ return ftxui::text(""); };
  qNavMenuOption.entries_option.transform = [](const ftxui::EntryState& state) {
    auto element = ftxui::text(" " + state.label + " ");
    if (state.focused) {
      element |= ftxui::inverted;
    }
    if (state.active) {
      element |= ftxui::bold;
    }
    return element | ftxui::flex;
  };
  qNavMenuOption.on_enter = [&]{ProcessingFuncs::OnSelectedQNavButton(context, qNavPaths[qNavSelected]);};
  ftxui::Component qNavMenu = ftxui::Menu(&qNavPaths, &qNavSelected, qNavMenuOption);


  //QuickNav add new entry button
  ftxui::Component qNavAddButton = ftxui::Button("Add Current Path", [&]{
      qNavPaths.push_back(context.currentPath);
      std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
    });


  int selected = context.currentContent.size() > 1 ? 1 : 0;
  auto menu_option = ftxui::MenuOption();
  menu_option.on_enter = [&]{ProcessingFuncs::OnSelectedMenuOption(context, selected);};
  ftxui::Component menu = ftxui::Menu(&context.currentStringified, &selected, menu_option);

  int sortSelected = 0;
  auto sort_menu_option = ftxui::MenuOption::Toggle();
  sort_menu_option.entries_option.transform = [](const ftxui::EntryState& state) {
    auto element = ftxui::text(state.label);
    if (state.focused) {
      element |= ftxui::inverted;
    }
    if (state.active) {
      element |= ftxui::bold;
    }
    return element | ftxui::center | ftxui::flex;
  };
  std::vector<std::string> sortOptions = {"Name Ascending", "Name Descending", "Last Modified Ascending", "Last Modified Descending"};
  sort_menu_option.on_change = [&]{context.sortType = SortTypes(sortSelected); ProcessingFuncs::OnSelectedSortOption(context);};
  ftxui::Component sort = ftxui::Menu(&sortOptions, &sortSelected, sort_menu_option);


  auto sortBox = ftxui::Renderer(sort , [&] {
    return ftxui::window(ftxui::text("Sort"),
          sort->Render() | ftxui::xframe
        );
  });

  auto menuBox = ftxui::Renderer(menu , [&] {
    return ftxui::window(ftxui::text("Content"),
          menu->Render() | ftxui::yframe
        ) | ftxui::flex;
  });

  auto quickNavBox = ftxui::Renderer(qNavMenu, [&] {
    return ftxui::vbox(
        qNavMenu->Render()
      ) | ftxui::flex;
    });

  auto quickNavSeparator = ftxui::Renderer([&] {
    return ftxui::separator();
  });

  auto quickNavAddBox = ftxui::Renderer(qNavAddButton, [&] {
    return ftxui::vbox(
        qNavAddButton->Render()
      );
    });

  auto locationBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Location"),
      ftxui::text(context.currentPath) | ftxui::bold
    );
  });

  auto previewBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Item Preview"),
      ftxui::text("item preview placeholder")
    ) | ftxui::flex;
  });

  auto exceptionBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Exception"),
      ftxui::text(exception) | ftxui::bold
    );
  });

  int selectedLeftChild = 0;
  auto leftPaneContainer = ftxui::Container::Vertical({
    quickNavBox,
    quickNavSeparator,
    quickNavAddBox
  }, &selectedLeftChild);

  auto leftPane = ftxui::Renderer(leftPaneContainer, [&] {
    return ftxui::window(ftxui::text("Quick Navigation"),
      leftPaneContainer->Render()
    );
  });

  int selectedMiddleChild = 2;
  auto middlePane = ftxui::Container::Vertical({
    locationBox,
    sortBox,
    menuBox,
    exceptionBox
  }, &selectedMiddleChild) | ftxui::flex;

  auto rightPane = ftxui::Container::Vertical({
    previewBox
  }) | ftxui::flex;

  int selectedFinalChild = 1;
  auto compositionFinal = ftxui::Container::Horizontal({
    leftPane, 
    middlePane, 
    rightPane}, 
    &selectedFinalChild);

  auto layout = ftxui::CatchEvent(compositionFinal, [&](ftxui::Event event) {
        if (event == ftxui::Event::Tab) {
            if(selectedFinalChild == 0){
                selectedFinalChild = 1;
                return true;
            }
             if(selectedFinalChild == 1){
                if(selectedMiddleChild == 1){
                    selectedMiddleChild = 2;
                    return true;
                }
                if(selectedMiddleChild == 2){
                    selectedFinalChild = 2;
                    return true;
                }
                return false;
            }
            if(selectedFinalChild == 2){
                selectedFinalChild = 0;
                return true;
            }
        }
        if (event == ftxui::Event::TabReverse) {
            if(selectedFinalChild == 0){
                selectedFinalChild = 1;
                return true;
            }
             if(selectedFinalChild == 1){
                if(selectedMiddleChild == 0){
                    selectedMiddleChild = 1;
                    return true;
                }
                if(selectedMiddleChild == 1){
                    selectedFinalChild = 0;
                    return true;
                }
                return false;
            }
            if(selectedFinalChild == 2){
                selectedFinalChild = 0;
                return true;
            }
        }
        if(event == ftxui::Event::Delete && selectedFinalChild == 0 && selectedLeftChild == 0){
            qNavPaths.erase(qNavPaths.begin() + qNavSelected);
            context.data["qNavEntries"] = qNavPaths;
            std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
            return true;
        }
        return false;
    });

  screen.Loop(layout);
}