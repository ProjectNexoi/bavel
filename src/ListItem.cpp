#include <iostream>
#include <string>
#include <ctime>
#include "Header.hpp"


ListItem::ListItem(ItemTypes t, std::string n, std::filesystem::file_time_type l){
    this->type = t;
    this->name = n;
    this->lastOpened = l;
}

ListItem::ListItem(ItemTypes type, std::string name){
    this->type = type;
    this->name = name;
}

void ListItem::SetType(ItemTypes t){
    this->type = t;
}

ItemTypes ListItem::GetType(){
    return this->type;
}

void ListItem::SetName(std::string n){
    this->name = n;
}

std::string ListItem::GetName(){
    return this->name;
}

void ListItem::SetLastOpened(std::filesystem::file_time_type l){
    this->lastOpened = l;
}

std::filesystem::file_time_type ListItem::GetLastOpened(){
    return this->lastOpened;
}

std::string ListItem::ToString(){
    std::string typeArr[3] = {"DIR", "FIL",""};
    std::string time = "a";
    try{  
        time = (this->type == 2 ? "" : ProcessingFuncs::FsTimeToString(this->lastOpened));
    }
    catch(std::filesystem::filesystem_error &e){}
    return typeArr[this->type] + "   " + time + "   " + this->name;
}
