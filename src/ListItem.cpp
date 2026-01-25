#include <iostream>
#include <string>
#include <ctime>
#include "Header.hpp"
#include <sys/stat.h>
#include <pwd.h>
#include <filesystem>
#include <unistd.h>
namespace fs = std::filesystem;

ListItem::ListItem(ItemTypes t, std::string p, std::filesystem::file_time_type l){
    this->type = t;
    this->path = p;
    this->fileName = p.substr(p.find_last_of("/")+1);
    this->lastOpened = l;
}

ListItem::ListItem(ItemTypes type, std::string path){
    this->type = type;
    this->path = path;
    this->fileName = path.substr(path.find_last_of("/")+1);
}

ListItem::ListItem(fs::directory_entry entry){
    std::error_code ec;
    this->wasSizeFetched = false;
    this->wasOwnerFetched = false;
    this->size = 0;
    this->path = entry.path().string();
    this->fileName = entry.path().filename().string();
    this->type = entry.is_directory() ? ItemTypes::DIR : ItemTypes::FIL;
    try{this->lastOpened = entry.last_write_time();} catch(fs::filesystem_error &e){}
    try{

        if(entry.is_symlink() ){
            this->wasSizeFetched = true;
            this->wasOwnerFetched = true;
        }
        if(this->type == ItemTypes::FIL){
            this->size = entry.file_size();
            this->wasSizeFetched = true;
        }
    } catch(fs::filesystem_error &e){}
}

void ListItem::SetType(ItemTypes t){
    this->type = t;
}

ItemTypes ListItem::GetType(){
    return this->type;
}

void ListItem::SetPath(std::string p){
    this->path = p;
    this->fileName = p.substr(p.find_last_of("/")+1);
}

std::string ListItem::GetPath(){
    return this->path;
}

std::string ListItem::GetFileName(){
    return this->fileName;
}

void ListItem::SetFileName(std::string fn){
    this->fileName = fn;
}

void ListItem::SetLastOpened(std::filesystem::file_time_type l){
    this->lastOpened = l;
}

std::filesystem::file_time_type ListItem::GetLastOpened(){
    return this->lastOpened;
}

void ListItem::SetSize(uintmax_t s){
    this->size = s;
}

uintmax_t ListItem::GetSize(){
    if(!this->wasSizeFetched){
        if (this->type == ItemTypes::DIR) {
            if (access(this->path.c_str(), R_OK | X_OK) != 0) {
                this->size = 0;
                this->wasSizeFetched = true;
                return this->size;
            }
            else{
                try{
                this->size = GetDirectorySize(fs::directory_entry(this->path));
                this->wasSizeFetched = true;
                } catch(fs::filesystem_error &e){}
            }
        }
    }
    return this->size;
}

void ListItem::SetOwner(std::string o){
    this->owner = o;
}

std::string ListItem::GetOwner(){
    if(!this->wasOwnerFetched){
        struct stat info;
        if (stat(path.c_str(), &info) == 0) {
            struct passwd *pw = getpwuid(info.st_uid);
            if (pw != nullptr) {
                this->owner = pw->pw_name;
                this->wasOwnerFetched = true;
            } else {
                this->owner = "Unknown";
                this->wasOwnerFetched = true;
            }
        } else {
            this->owner = "Error";
            this->wasOwnerFetched = true;
        }
    }
    return this->owner;
}

std::string ListItem::ToString(){
    std::string typeArr[3] = {"DIR", "FIL",""};
    return typeArr[this->type] 
    + "   "
    + (this->type == 2 ? "" : ProcessingFuncs::FsTimeToString(this->lastOpened)) 
    + "   " 
    + this->fileName;
}
