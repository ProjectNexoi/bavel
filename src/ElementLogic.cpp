#include "Header.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

namespace fs = std::filesystem;

namespace ElementLogic {
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
              ProcessingFuncs::StringifyContent(context);
              selected = 0;
            } 
            else if(context.currentContent[selected]->GetType() == ItemTypes::DIR){
              pathDestination = context.currentContent[selected]->GetPath();
              PathToItemList(pathDestination, context);
              SortItemList(context);
              context.currentPath = pathDestination;
              ProcessingFuncs::StringifyContent(context);
              selected = context.currentContent.size() > 1 ? 1 : 0;
            } 
            else if(context.currentContent[selected]->GetType() == ItemTypes::FIL){
              std::system(("xdg-open '" + context.currentContent[selected]->GetPath() + "'").c_str() );
            }
          }
          catch(fs::filesystem_error &e){
            context.exception = e.what();
          }
    }

    void OnSelectedSortOption(Context& context){
      SortItemList(context);
      context.data["sortType"] = context.sortType;
      std::ofstream(std::string(context.homedir) + "/.bavel/data.json") << context.data;
      ProcessingFuncs::StringifyContent(context);
    }

    void OnSelectedQNavButton(Context& context, int qNavSelected){
      try{
        context.exception = "";
        PathToItemList(context.qNavPaths[qNavSelected], context);
        SortItemList(context);
        context.currentPath = context.qNavPaths[qNavSelected];
        ProcessingFuncs::StringifyContent(context);
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

    void OnSelectedNewDirectoryButton(Context& context, std::string name){
      if(name == ""){
        return;
      }
      try{
        fs::create_directory(context.currentPath + "/" + name);
        ProcessingFuncs::ReloadItemList(context);
        context.anyModalActive = false;
      }
      catch(fs::filesystem_error &e){
        context.exception = e.what();
      }
    }

    void OnSelectedNewFileButton(Context& context, std::string name){
      if(name == ""){
        return;
      }
      try{
        std::ofstream ofs(context.currentPath + "/" + name);
        ofs.close();
        ProcessingFuncs::ReloadItemList(context);
        context.anyModalActive = false;
      }
      catch(fs::filesystem_error &e){
        context.exception = e.what();
      }
    }
    void OnSelectedDeleteElementButton(Context& context, int& selected){
      if(context.currentContent[selected]->GetType() == ItemTypes::DIR){
        try{
          fs::remove_all(context.currentContent[selected]->GetPath());
          ProcessingFuncs::ReloadItemList(context);
          context.anyModalActive = false;
        }
        catch(fs::filesystem_error &e){
          context.exception = e.what();
        }
      }
      else if(context.currentContent[selected]->GetType() == ItemTypes::FIL){
        try{
          fs::remove(context.currentContent[selected]->GetPath());
          ProcessingFuncs::ReloadItemList(context);
          context.anyModalActive = false;
        }
        catch(fs::filesystem_error &e){
          context.exception = e.what();
        }
      }
    }
}