#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <map>
#include <array>
#include <memory>

using namespace std;
using namespace rapidjson;

using settings_type = map<string, map<string, array<const int, 3>>>;

settings_type load_settings();

int main()
{
    int path_min[3];
    int path_max[3];

    auto settings = load_settings();



    ifstream fs("setting.json");
    IStreamWrapper isw(fs);

    Document d;
    d.ParseStream(isw);

    const Value& val = d["rgb"];

    return 0;
}

settings_type load_settings()
{
    settings_type settings;

    Document d;
    ifstream fs("setting.json");
    IStreamWrapper isw(fs);

    d.ParseSream(isw);
    const Value& val = d["rgb"];

    for (auto& setting: val.GetObject()) {
        settings_type::mapped_type subsettings;
        
        for (auto& subsetting: setting.GetObject()) {
            auto& subsetting_name = subsetting.name.GetString();
            array<const int, 3> rgb;

            for (SizeType i = 0; i != 3; ++i)
                rgb[i] = subsetting.GetInt();

            subsettings[subsetting_name] = std::move(rgb);
        }
        settings[setting.name.GetString()] = std::move(subsettings);
    }

    return settings;
}
