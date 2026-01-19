#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>
#include <chrono>
//namespace fs = std::filesystem;

#include "Header.hpp"

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

  //Sets window to fill terminal
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  auto screen = ftxui::ScreenInteractive::FixedSize(size.ws_col,size.ws_row);

  SortTypes sortType = SortTypes::NAME_ASC;

  //Fetches the current directory's content
  std::string currentPath = "/";
  std::vector<ListItem*> currentContent;
  PathToItemList(currentPath, currentContent);
  SortItemList(currentContent, sortType);

  std::vector<std::string> currentStringified;
  ProcessingFuncs::StringifyContent(currentContent, currentStringified);

  int selected = currentContent.size() > 1 ? 1 : 0;
  auto menu_option = ftxui::MenuOption();
  std::string exception = "";
  menu_option.on_enter = [&]{ProcessingFuncs::OnSelectedMenuOption(currentContent, currentStringified, currentPath, selected, exception, sortType); selected = currentContent.size() > 1 ? 1 : 0;};
  ftxui::Component menu = ftxui::Menu(&currentStringified, &selected, menu_option);

  int sortSelected = 0;
  auto sort_menu_option = ftxui::MenuOption::Toggle();
  sort_menu_option.underline.enabled = true;
  std::vector<std::string> sortOptions = {"Name Ascending", "Name Descending", "Last Modified Ascending", "Last Modified Descending"};
  sort_menu_option.on_change = [&]{sortType = SortTypes(sortSelected); ProcessingFuncs::OnSelectedSortOption(currentContent,currentStringified,sortType); selected = currentContent.size() > 1 ? 1 : 0;};
  ftxui::Component sort = ftxui::Menu(&sortOptions, &sortSelected, sort_menu_option);

  auto sortBox = ftxui::Renderer(sort , [&] {
    return ftxui::window(ftxui::text("Sort"),
          sort->Render() | ftxui::yframe
        );
  });

  auto menuBox = ftxui::Renderer(menu , [&] {
    return ftxui::window(ftxui::text("Content"),
          menu->Render() | ftxui::yframe
        ) | ftxui::flex;
  });

  auto quickNavBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Quick Navigation"),
        ftxui::vbox({
          ftxui::text("nav")
        })

      ) | ftxui::flex;
    });

  auto locationBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Location"),
      ftxui::text(currentPath) | ftxui::bold
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

  auto leftPane = ftxui::Container::Vertical({
    quickNavBox
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
                    selectedMiddleChild = 1;
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
                if(selectedMiddleChild == 1){
                    selectedMiddleChild = 2;
                    return true;
                }
                if(selectedMiddleChild == 2){
                    selectedMiddleChild = 1;
                    return true;
                }
                return false;
            }
            if(selectedFinalChild == 2){
                selectedFinalChild = 0;
                return true;
            }
        }
        return false;
    });

  screen.Loop(layout);
}