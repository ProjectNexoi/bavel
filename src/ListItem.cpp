#include <iostream>
#include <string>
#include <ctime>
#include "Header.hpp"


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

std::string ListItem::ToString(){
    std::string typeArr[3] = {"DIR", "FIL",""};
    return typeArr[this->type] 
    + "   "
    + (this->type == 2 ? "" : ProcessingFuncs::FsTimeToString(this->lastOpened)) 
    + "   " 
    + this->fileName;
}
