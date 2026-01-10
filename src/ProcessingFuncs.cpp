#include "Header.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace ProcessingFuncs {
    void StringifyContent(std::vector<ListItem*>& currentContent, std::vector<std::string>& currentStringified) {
        for(int i = 0; i < currentContent.size(); i++){
            currentStringified.push_back(currentContent[i]->ToString());
        }
    }

    void OnSelectedMenuOption(
        std::vector<ListItem*>& currentContent, 
        std::vector<std::string>& currentStringified,
        std::string& currentPath,
        int& selected,
        std::string& exception) {
        try{
            exception = "";
            std::string pathDestination;
            if(currentContent[selected]->GetType() == ItemTypes::BACK){
              pathDestination = currentPath.substr(0, currentPath.find_last_of("/"));
              if(pathDestination == ""){
                pathDestination = "/";
              }
              PathToItemList(pathDestination, currentContent);
              SortItemList(currentContent);
              currentPath = pathDestination;
              currentStringified.clear();
              for(int i = 0; i < currentContent.size(); i++){
                currentStringified.push_back(currentContent[i]->ToString());
              }
            } 
            else if(currentContent[selected]->GetType() == ItemTypes::DIR){
              pathDestination = currentContent[selected]->GetName();
              PathToItemList(pathDestination, currentContent);
              SortItemList(currentContent);
              currentPath = pathDestination;
              currentStringified.clear();
              for(int i = 0; i < currentContent.size(); i++){
                currentStringified.push_back(currentContent[i]->ToString());
              }
              selected = 0;
            } 
            else if(currentContent[selected]->GetType() == ItemTypes::FIL){
              std::system(("xdg-open '" + currentContent[selected]->GetName() + "'").c_str() );
            }
          }
          catch(fs::filesystem_error &e){
            exception = e.what();
          }
    }

    std::string FsTimeToString(fs::file_time_type time){
      auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
      );

      // Convert to time_t (C-style time)
      std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

      // Convert to string
      std::string timeStr = std::ctime(&cftime);

      // Remove the newline added by ctime
      if (!timeStr.empty() && timeStr.back() == '\n') {
          timeStr.pop_back();
      }
      return timeStr;
    }

    std::time_t FsTimeToTimeT(fs::file_time_type time){
      auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
      );

      // Convert to time_t (C-style time)
      std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

      return cftime;
    }

    /*
    ftxui::Elements GetLayout(ftxui::Component& menu){
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
    }
    */
}

