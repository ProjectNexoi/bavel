#include "Header.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

namespace fs = std::filesystem;

namespace ProcessingFuncs {
    void StringifyContent(Context& context) {
      context.currentStringified.clear();
        for(int i = 0; i < context.currentContent.size(); i++){
            context.currentStringified.push_back(context.currentContent[i]->ToString());
        }
    }

    void OnSelectedMenuOption(
        Context& context,
        int& selected) {
        try{
            context.exception = "";
            std::string pathDestination;
            if(context.currentContent[selected]->GetType() == ItemTypes::BACK){
              pathDestination = context.currentPath.substr(0, context.currentPath.find_last_of("/"));
              //clamps pathDestination to root
              if(pathDestination == ""){
                pathDestination = "/";
              }
              PathToItemList(pathDestination, context);
              SortItemList(context);
              context.currentPath = pathDestination;
              StringifyContent(context);
              selected = 0;
            } 
            else if(context.currentContent[selected]->GetType() == ItemTypes::DIR){
              pathDestination = context.currentContent[selected]->GetName();
              PathToItemList(pathDestination, context);
              SortItemList(context);
              context.currentPath = pathDestination;
              StringifyContent(context);
              selected = context.currentContent.size() > 1 ? 1 : 0;
            } 
            else if(context.currentContent[selected]->GetType() == ItemTypes::FIL){
              std::system(("xdg-open '" + context.currentContent[selected]->GetName() + "'").c_str() );
            }
          }
          catch(fs::filesystem_error &e){
            context.exception = e.what();
          }
    }

    void OnSelectedSortOption(Context& context){
      SortItemList(context);
      StringifyContent(context);
    }

    void OnSelectedQNavButton(Context& context, std::string pathDestination){
      try{
        context.exception = "";
        PathToItemList(pathDestination, context);
        SortItemList(context);
        context.currentPath = pathDestination;
        StringifyContent(context);
      }
      catch(fs::filesystem_error &e){
        context.exception = e.what();
      }
    }

    void OnSelectedQNavAddButton(Context& context){
      try{
        context.exception = "";
        if(!context.data["qNavEntries"].is_array()){
           context.data["qNavEntries"] = nlohmann::json::array();
        }
        context.data["qNavEntries"].push_back(context.currentPath);
        std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
      }
      catch(const fs::filesystem_error &e){
        context.exception = e.what();
      }
      catch(const std::exception &e){
        context.exception = std::string("Error: ") + e.what();
      }
      catch(...){
        context.exception = "Unknown error occurred";
      }
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