#include "Header.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace ProcessingFuncs {
    void StringifyContent(std::vector<ListItem*>& currentContent, std::vector<std::string>& currentStringified) {
      currentStringified.clear();
        for(int i = 0; i < currentContent.size(); i++){
            currentStringified.push_back(currentContent[i]->ToString());
        }
    }

    void OnSelectedMenuOption(
        std::vector<ListItem*>& currentContent, 
        std::vector<std::string>& currentStringified,
        std::string& currentPath,
        int& selected,
        std::string& exception,
        SortTypes& sortType) {
        try{
            exception = "";
            std::string pathDestination;
            if(currentContent[selected]->GetType() == ItemTypes::BACK){
              pathDestination = currentPath.substr(0, currentPath.find_last_of("/"));
              //clamps pathDestination to root
              if(pathDestination == ""){
                pathDestination = "/";
              }
              PathToItemList(pathDestination, currentContent);
              SortItemList(currentContent, sortType);
              currentPath = pathDestination;
              StringifyContent(currentContent, currentStringified);
            } 
            else if(currentContent[selected]->GetType() == ItemTypes::DIR){
              pathDestination = currentContent[selected]->GetName();
              PathToItemList(pathDestination, currentContent);
              SortItemList(currentContent, sortType);
              currentPath = pathDestination;
              StringifyContent(currentContent, currentStringified);
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

    void OnSelectedSortOption(std::vector<ListItem*>& currentContent, std::vector<std::string>& currentStringified, SortTypes& sortType){
      SortItemList(currentContent, sortType);
      StringifyContent(currentContent, currentStringified);
    }


    //Horrible, horrible, horrible.
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
}