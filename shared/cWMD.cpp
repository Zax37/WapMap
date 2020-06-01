#include "cWMD.h"
#include "cWWD.h"

void WMD::ExportTileProperties(WWD::Parser *hParser, std::ofstream& ofstream) {
    for (size_t z = 0; z < hParser->m_hTileAttribs.size(); z++) {
        ofstream << z << ": " << hParser->m_hTileAttribs[z]->toString() << '\n';
    }
}
