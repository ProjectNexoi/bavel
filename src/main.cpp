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

  //Fetches the current directory's content
  std::string currentPath = "/";
  std::vector<ListItem*> currentContent;
  PathToItemList(currentPath, currentContent);

  std::vector<std::string> currentStringified;
  ProcessingFuncs::StringifyContent(currentContent, currentStringified);

  int selected = 1;
  auto menu_option = ftxui::MenuOption();
  std::string exception = "";

  menu_option.on_enter = [&]{ProcessingFuncs::OnSelectedMenuOption(currentContent, currentStringified, currentPath, selected, exception); selected = 1;};

  ftxui::Component menu = ftxui::Menu(&currentStringified, &selected, menu_option);

  auto renderer = ftxui::Renderer(menu, [&] {
    
    return ftxui::hbox({

      ftxui::window(ftxui::text("Quick Navigation"),
        ftxui::vbox({
          ftxui::text("nav")
        })

      ),
      
      ftxui::vbox(

        ftxui::window(ftxui::text("Location"),
          ftxui::text(currentPath) | ftxui::bold
        ),

        ftxui::window(ftxui::text("Content"),
          menu->Render() | ftxui::yframe
        ) | ftxui::flex,

        ftxui::window(ftxui::text("Exception"),
          ftxui::text(exception) | ftxui::bold
        )

      ) | ftxui::flex,

      ftxui::window(ftxui::text("Item Preview"),
        ftxui::text("item preview placeholder")
      ) | ftxui::flex,

    });
    
  });
  
  screen.Loop(renderer);
}