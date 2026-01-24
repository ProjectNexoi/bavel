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

    void ParseQNavPathsToEntries(Context& context) {
      context.qNavEntries.clear();
      for(int i = 0; i < context.qNavPaths.size(); i++){
        std::string path = context.qNavPaths[i];
        if(path.rfind(context.homedir, 0) == 0){
          path.erase(0, context.homedir.length());
          path = "~" + path;
        }
        if(path.length() > 32){
          path = path.substr(0, 14) + " ... " + path.substr(path.length() - 13, path.length());
        }
        context.qNavEntries.push_back(path);
      }
    }

    void ParseCurrentPathToNavText(Context& context){
      context.locationBarText = context.currentPath;
      if(context.locationBarText.rfind(context.homedir, 0) == 0){
        context.locationBarText.erase(0, context.homedir.length());
        context.locationBarText = "~" + context.locationBarText;
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

    void ReloadItemList(Context& context){
      PathToItemList(context.currentPath, context);
      SortItemList(context);
      ProcessingFuncs::StringifyContent(context);
    }
}