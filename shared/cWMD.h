#ifndef WAPMAP_CWMD_H
#define WAPMAP_CWMD_H

#include <fstream>

namespace WWD {
    class Parser;
}

namespace WMD {
    void ExportTileProperties(WWD::Parser* hParser, std::ofstream& ofstream);
};

#endif //WAPMAP_CWMD_H
