#include <ftxui/dom/flexbox_config.hpp>
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
  // Initialize context
  Context context;

  // Sets window to fill terminal
  /* 
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size); 
  */
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  // Get user's home directory
  struct passwd *pw = getpwuid(getuid());
  context.homedir = pw->pw_dir;

  // Fetch json data
  if(!fs::exists(std::string(context.homedir) + "/.bavel/data.json")){ // If no data, then make the file
    fs::create_directories(std::string(context.homedir) + "/.bavel");
    std::ofstream(std::string(context.homedir) + "/.bavel/data.json");
  }
  if(!fs::is_empty(std::string(context.homedir) + "/.bavel/data.json")){
    std::ifstream(std::string(context.homedir) + "/.bavel/data.json") >> context.data;
  }

  try{context.sortType = SortTypes(context.data["sortType"].get<int>());}catch(std::exception& e){
    context.data["sortType"] = context.sortType;
    std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
  }

  
  // Fetches the current directory's content
  context.currentPath = context.homedir;
  NavigateToPath(context, context.currentPath);
  int selected = context.currentContent.size() > 1 ? 1 : 0;
  ElementLogic::UpdateInformationBox(context, selected);

  // QuickNav entries
  try{context.qNavPaths = context.data["qNavEntries"].get<std::vector<std::string>>();}
  catch(std::exception& e){context.exception = e.what();}
  ProcessingFuncs::ParseQNavPathsToEntries(context);
  int qNavSelected = 0;
  auto qNavMenuOption = ftxui::MenuOption(ftxui::MenuOption::Vertical());
  qNavMenuOption.elements_prefix = []{ 
    return ftxui::text("                                  "); }; // So that the leftpane doesn't change widths whenever a user adds/deletes a quicknav entry 
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
  qNavMenuOption.on_enter = [&]{ElementLogic::OnSelectedQNavButton(context, qNavSelected); ProcessingFuncs::ParseQNavPathsToEntries(context);};
  ftxui::Component qNavMenu = ftxui::Menu(&context.qNavEntries, &qNavSelected, qNavMenuOption);


  // QuickNav add new entry button
  ftxui::Component qNavAddButton = ftxui::Button("Add Current Path", [&]{
      context.qNavPaths.push_back(context.currentPath);
      context.data["qNavEntries"] = context.qNavPaths;
      std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
      ProcessingFuncs::ParseQNavPathsToEntries(context);
  });

  // Initialize file menu
  auto menu_option = ftxui::MenuOption();
  menu_option.on_enter = [&]{ElementLogic::OnSelectedMenuOption(context, selected);};
  menu_option.on_change = [&]{ElementLogic::UpdateInformationBox(context, selected);};
  ftxui::Component menu = ftxui::Menu(&context.currentStringified, &selected, menu_option);

  // Initialize sort menu
  int sortSelected = context.sortType;
  auto sort_menu_option = ftxui::MenuOption::Toggle();
  sort_menu_option.entries_option.transform = [](const ftxui::EntryState& state) {
    auto element = ftxui::text(state.label);
    if (state.focused) {
      element |= ftxui::inverted;
    }
    if (state.active) {
      element |= ftxui::bold;
    }
    return element | ftxui::hcenter | ftxui::flex;
  };
  std::vector<std::string> sortOptions = {"Name Ascending", "Name Descending", "Last Modified Ascending", "Last Modified Descending"};
  sort_menu_option.on_change = [&]{context.sortType = SortTypes(sortSelected); ElementLogic::OnSelectedSortOption(context);};
  ftxui::Component sort = ftxui::Menu(&sortOptions, &sortSelected, sort_menu_option);


  // GUI starts here
  
  auto centered_button_option = ftxui::ButtonOption::Border();
  centered_button_option.transform = [](const ftxui::EntryState& state) {
    auto element = ftxui::text(state.label) | ftxui::center;
    element |= ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 20);
    element |= ftxui::border;
    if (state.focused) {
      element |= ftxui::inverted;
    }
    return element | ftxui::xflex;
  };

  // New File/Dir Logic & GUI
  std::string newElementName = "";
  auto newElementNameInput = ftxui::Input(&newElementName, ftxui::InputOption{.placeholder = " Enter Name"}) | ftxui::border;
  ftxui::Component newFileButton = ftxui::Button("File", [&]{ElementLogic::OnSelectedNewFileButton(context, newElementName); newElementName = "";}, centered_button_option);
  ftxui::Component newDirectoryButton = ftxui::Button("Directory", [&]{ElementLogic::OnSelectedNewDirectoryButton(context, newElementName); newElementName = "";}, centered_button_option);
  ftxui::Component newElementCancelButton = ftxui::Button("Cancel", [&]{context.anyModalActive = false; newElementName = "";}, centered_button_option);

  int newElementSelected = 4;
  auto newElementContent = ftxui::Container::Vertical({
    ftxui::Renderer([] { return ftxui::text("Create New") | ftxui::center | ftxui::bold; }),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([] {return ftxui::text("This action will create a new element in the current directory named:") | ftxui::center;}),
    ftxui::Renderer([] {return ftxui::text("");}),
    newElementNameInput,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Renderer([&] {return ftxui::text("Of type:") | ftxui::center;}),
    ftxui::Container::Horizontal({
      newFileButton,
      newDirectoryButton}),
    newElementCancelButton
    }, &newElementSelected) | ftxui::flex;

  auto newElementUI = ftxui::Renderer(newElementContent, [&] {
    return ftxui::window(ftxui::text(""),
      newElementContent->Render()
    ) | ftxui::size(ftxui::WidthOrHeight::HEIGHT, ftxui::Constraint::EQUAL, 16)
      | ftxui::size(ftxui::WidthOrHeight::WIDTH, ftxui::Constraint::EQUAL, 75)
      | ftxui::center
      | ftxui::clear_under;
  });

  // Delete Element Logic & GUI
  ftxui::Component deleteElementButton = ftxui::Button("Delete", [&]{ElementLogic::OnSelectedDeleteElementButton(context, selected);}, centered_button_option);
  ftxui::Component deleteElementCancelButton = ftxui::Button("Cancel", [&]{context.anyModalActive = false;}, centered_button_option);
  int deleteElementSelected = 7;
  std::string typeDeleteArr[2] = {"DIRECTORY", "FILE"};
  auto deleteElementContent = ftxui::Container::Vertical({
    ftxui::Renderer([] {return ftxui::text("Element Deletion") | ftxui::center | ftxui::bold; }),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([] {return ftxui::hbox(
      ftxui::text("This action will delete the following and "), 
      ftxui::text("cannot be undone") | ftxui::underlinedDouble,
      ftxui::text(":")
      ) | ftxui::center;}),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([&] {return ftxui::text(context.currentContent[selected]->GetFileName()) | ftxui::center | ftxui::bold;}),
    ftxui::Renderer([&] {return ftxui::hbox(
      ftxui::text("of type "), 
      ftxui::text(typeDeleteArr[context.currentContent[selected]->GetType()])| ftxui::bold | 
      ftxui::color(ftxui::Color::Red)) | ftxui::center;}),
    ftxui::Renderer([] {return ftxui::filler();}),
    deleteElementCancelButton,
    deleteElementButton
  }, &deleteElementSelected) | ftxui::flex;
  

  auto deleteElementUI = ftxui::Renderer(deleteElementContent, [&] {
    return ftxui::window(ftxui::text(""),
      deleteElementContent->Render()
    ) | ftxui::size(ftxui::WidthOrHeight::HEIGHT, ftxui::Constraint::EQUAL, 15)
      | ftxui::size(ftxui::WidthOrHeight::WIDTH, ftxui::Constraint::EQUAL, 65)
      | ftxui::center
      | ftxui::clear_under;
  });

  // Rename Element Logic & GUI
  int renameElementSelected = 7;
  std::string renameNewName = "";
  auto renameElementNameInput = ftxui::Input(&renameNewName, ftxui::InputOption{.placeholder = " Enter New Name"}) | ftxui::border;
  ftxui::Component renameElementButton = ftxui::Button("Rename", [&]{ElementLogic::OnSelectedRenameElementButton(context, selected, renameNewName); renameNewName = "";}, centered_button_option);
  ftxui::Component renameElementCancelButton = ftxui::Button("Cancel", [&]{context.anyModalActive = false; renameNewName = "";}, centered_button_option);

  auto renameElementContent = ftxui::Container::Vertical({
    ftxui::Renderer([] {return ftxui::text("Rename Element") | ftxui::center | ftxui::bold; }),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([] {return ftxui::hbox( ftxui::text("This element will be renamed from: ")) | ftxui::center;}),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([&] {return ftxui::text(context.currentContent[selected]->GetFileName()) | ftxui::center | ftxui::bold;}),
    ftxui::Renderer([] {return ftxui::text("");}),
    ftxui::Renderer([] {return ftxui::text("To:") |ftxui::center;}),
    renameElementNameInput,
    ftxui::Renderer([] {return ftxui::filler();}),
    renameElementButton | ftxui::flex,
    renameElementCancelButton | ftxui::flex
  }, &renameElementSelected) | ftxui::flex;

  auto renameElementUI = ftxui::Renderer(renameElementContent, [&] {
    return ftxui::window(ftxui::text(""),
      renameElementContent->Render()
    ) | ftxui::size(ftxui::WidthOrHeight::HEIGHT, ftxui::Constraint::EQUAL, 18)
      | ftxui::size(ftxui::WidthOrHeight::WIDTH, ftxui::Constraint::EQUAL, 60)
      | ftxui::center
      | ftxui::clear_under;
  });

  // Item Info Logic & GUI

  auto ItemInfoContent = ftxui::Container::Vertical({
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Name: ");}),
      ftxui::Renderer([&] {return ftxui::text(context.metadataContext.itemName);})
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Type: ");}),
      ftxui::Renderer([&] {return ftxui::text(context.metadataContext.itemType);})
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Last Write: ");}),
      ftxui::Renderer([&] {return ftxui::text(context.metadataContext.itemLastWrite);})
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Size: ");}),
      ftxui::Renderer([&] {
        if (!context.metadataContext.itemSize.valid()) {
          return ftxui::text("N/A");
        }
        // Non-blocking check for availability
        if (context.metadataContext.itemSize.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
           return ftxui::text(context.metadataContext.itemSize.get());
        } else {
           return ftxui::text("Calculating...");
        }
      })
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Owner: ");}),
      ftxui::Renderer([&] {return ftxui::text(context.metadataContext.itemOwner);})
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();}),
    ftxui::Container::Horizontal({
      ftxui::Renderer([] {return ftxui::text("Path: ");}),
      ftxui::Renderer([&] {return ftxui::text(context.metadataContext.itemPath);})
    }) | ftxui::center | ftxui::bold,
    ftxui::Renderer([] {return ftxui::filler();})
  });
    

  auto sortBox = ftxui::Renderer(sort , [&] {
    return ftxui::window(ftxui::text("Sort"),
          sort->Render() | ftxui::xflex
        ) | ftxui::xflex;
  });

  auto menuBox = ftxui::Renderer(menu , [&] {
    return ftxui::window(ftxui::text("Content"),
          menu->Render() | ftxui::yframe
        ) | ftxui::flex;
  });

  auto menuLayout = ftxui::CatchEvent(menuBox, [&](ftxui::Event event) {
    bool isNotBack = context.currentContent[selected]->GetType() != ItemTypes::BACK;
    if(event == ftxui::Event::Character("e") && isNotBack){
      screen.WithRestoredIO([&] {
          std::system(("vim '" + context.currentContent[selected]->GetPath() + "'").c_str() );
      })();
      return true;
    }
    if (event == ftxui::Event::Character("c")) {
      context.anyModalActive = true;
      context.activeModalIndex = 0;
      newElementUI->Focused();
      newElementSelected = 4;
      return true;
    }
    if (event == ftxui::Event::Character("d") && isNotBack){
      context.anyModalActive = true;
      context.activeModalIndex = 1;
      deleteElementUI->Focused();
      deleteElementSelected = 7;
      return true;
    }
    if (event == ftxui::Event::Character("r") && isNotBack){
      context.anyModalActive = true;
      context.activeModalIndex = 2;
      renameElementUI->Focused();
      renameElementSelected = 7;
      return true;
    }
    return false;
  });

  auto quickNavContentBox = ftxui::Renderer(qNavMenu, [&] {
    return ftxui::vbox(
      ftxui::vbox(
        qNavMenu->Render()
      ) | ftxui::flex,
      ftxui::separator(),
      ftxui::text("Highlight an entry") | ftxui::center,
      ftxui::text("and press [Enter]") | ftxui::center,
      ftxui::text("to navigate to it") | ftxui::center,
      ftxui::text("or press [Delete]") | ftxui::center,
      ftxui::text("to remove it") | ftxui::center
    ) | ftxui::flex;
    });

  auto quickNavContentLayout = ftxui::CatchEvent(quickNavContentBox, [&](ftxui::Event event) {
    if(event == ftxui::Event::Delete){
            context.qNavPaths.erase(context.qNavPaths.begin() + qNavSelected);
            context.data["qNavEntries"] = context.qNavPaths;
            std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
            ProcessingFuncs::ParseQNavPathsToEntries(context);
            return true;
        }
    return false;
  });

  auto quickNavSeparator = ftxui::Renderer([&] {
    return ftxui::separator();
  });

  auto quickNavAddBox = ftxui::Renderer(qNavAddButton, [&] {
    return ftxui::vbox(
        qNavAddButton->Render()
      );
  });

  auto locationBarOption = ftxui::InputOption::Default();

  auto locationBarInput = ftxui::Input(&context.locationBarText, "Enter Location");

  auto locationBox = ftxui::Renderer(locationBarInput, [&] {
    return ftxui::window(ftxui::text("Location"),
      locationBarInput->Render()
    );
  });

  auto locationLayout = ftxui::CatchEvent(locationBox, [&](ftxui::Event event) {
    if(event == ftxui::Event::Return){
      ElementLogic::OnLocationBarSubmit(context);
      return true;
    }
    return false;
  });

  auto ItemInfoBox = ftxui::Renderer(ItemInfoContent, [&] {
    return ftxui::window(ftxui::text("Item Information"),
      ItemInfoContent->Render()
    ) | ftxui::size(ftxui::WidthOrHeight::HEIGHT, ftxui::Constraint::EQUAL, 15);
  });

  auto exceptionBox = ftxui::Renderer([&] {
    return ftxui::window(ftxui::text("Exception"),
      ftxui::text(context.exception) | ftxui::bold
    );
  });

  int selectedLeftChild = 0;
  auto leftPaneContainer = ftxui::Container::Vertical({
    quickNavContentLayout,
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
    locationLayout,
    sortBox,
    menuLayout,
    exceptionBox
  }, &selectedMiddleChild) | ftxui::flex;

  auto rightPane = ftxui::Container::Vertical({
    ItemInfoBox
  }) | ftxui::flex;

  int selectedFinalChild = 1;
  auto mainUI = ftxui::Container::Horizontal({
    leftPane, 
    middlePane, 
    rightPane}, 
    &selectedFinalChild);

  auto mainLayout = ftxui::CatchEvent(mainUI, [&](ftxui::Event event) {
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
        return false;
  });

  auto mainLayoutDimmable = ftxui::Renderer(mainLayout, [&] {
    auto element = mainLayout->Render();
    if (context.anyModalActive) {
      element |= ftxui::dim;
    }
    return element;
  });

  auto modalContainer = ftxui::Container::Tab({
    newElementUI,
    deleteElementUI,
    renameElementUI
  }, &context.activeModalIndex);

  auto finalUI = ftxui::Modal(mainLayoutDimmable, modalContainer, &context.anyModalActive);

  screen.Loop(finalUI);
}