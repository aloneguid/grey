#pragma once
#include <string>
#include <unordered_map>
#include "app.h"
#include "widgets.h"
#include "fonts/MaterialIcons.h"

const std::unordered_map<std::string, std::string>& get_icon_map();

std::string get_icon(const std::string& icon_name);
