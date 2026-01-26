#include "Header.hpp"
#include <filesystem>
#include <future>
#include <thread>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unistd.h>

namespace fs = std::filesystem;

namespace ElementLogic {
    void OnSelectedMenuOption(
        Context& context,
        int& selected) {
          if (access(context.currentContent[selected]->GetPath().c_str(), R_OK | X_OK) != 0) {
            context.exception = "Permission denied";
            return;
          }
        try{
            context.exception = "";
            std::string pathDestination;
            if(context.currentContent[selected]->GetType() == ItemTypes::BACK){
              pathDestination = context.currentPath.substr(0, context.currentPath.find_last_of("/"));
              //clamps pathDestination to root
              if(pathDestination == ""){
                pathDestination = "/";
              }
              NavigateToPath(context, pathDestination);
              selected = 0;
            } 
            else if(context.currentContent[selected]->GetType() == ItemTypes::DIR){
              pathDestination = context.currentContent[selected]->GetPath();
              NavigateToPath(context,pathDestination);
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
      DataLoader::SaveDataToFile(context);
      ProcessingFuncs::StringifyContent(context);
    }

    void OnSelectedQNavButton(Context& context, int& qNavSelected){
      try{
        context.exception = "";
        NavigateToPath(context, context.qNavPaths[qNavSelected]);
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
        DataLoader::SaveDataToFile(context);
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

    void OnSelectedRenameElementButton(Context& context, int& selected, std::string& newName){
      if(newName == ""){
        return;
      }
      try{
        fs::rename(context.currentContent[selected]->GetPath(), context.currentPath + "/" + newName);
        ProcessingFuncs::ReloadItemList(context);
        context.anyModalActive = false;
      }
      catch(std::exception &e){
        context.exception = e.what();
      }
    }

    void OnLocationBarSubmit(Context& context){
      try{
        std::string path = context.locationBarText;
        if(path[0] == '~'){
          path = context.homedir + path.substr(1);
        }
        NavigateToPath(context, path);
      }
      catch(fs::filesystem_error &e){
        context.exception = e.what();
      }
    }

    void UpdateInformationBox(Context& context, int& selected){
      ListItem* item = context.currentContent[selected];
      bool isBack = false;
      if(item->GetType() == ItemTypes::BACK){
        item = new ListItem(fs::directory_entry(context.currentPath));
        isBack = true;
      }
      context.metadataContext.itemName = item->GetFileName();
      context.metadataContext.itemType = item->GetType() == ItemTypes::DIR ? "Directory" : "File";
      context.metadataContext.itemLastWrite = ProcessingFuncs::FsTimeToString(item->GetLastOpened());
      auto promise = std::make_shared<std::promise<std::uintmax_t>>();
      context.metadataContext.itemSize = promise->get_future().share();

      std::thread([promise, item, isBack]() {
          uintmax_t size = item->GetSize();
          if (isBack) {
              delete item;
          }
          promise->set_value(size);
      }).detach();

      context.metadataContext.itemOwner = item->GetOwner();
      context.metadataContext.itemPath = item->GetPath();
    }

    void OnQNavReorder(Context& context, int& selected, ReorderDirection direction){
      if(direction == ReorderDirection::UP){
        if(selected == 0){
          return;
        }
        std::swap(context.qNavPaths[selected], context.qNavPaths[selected - 1]);
        std::swap(context.qNavEntries[selected], context.qNavEntries[selected - 1]);
        selected--;
      }
      else if(direction == ReorderDirection::DOWN){
        if(selected == context.qNavPaths.size() - 1){
          return;
        }
        std::swap(context.qNavPaths[selected], context.qNavPaths[selected + 1]);
        std::swap(context.qNavEntries[selected], context.qNavEntries[selected + 1]);
        selected++;
      }
      context.data["qNavEntries"] = context.qNavPaths;
      DataLoader::SaveDataToFile(context);
    }
}