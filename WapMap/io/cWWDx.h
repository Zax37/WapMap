#ifndef IO_WWDX
#define IO_WWDX

#include "../../shared/cWWD.h"

struct DocumentData;

class cIO_WWDx : public WWD::CustomMetaSerializer {
private:
    DocumentData *hDocumentData;

    std::string ReadZeroTerminatedString(std::istream *hStream);

public:
    cIO_WWDx(DocumentData *dd);

    ~cIO_WWDx();

    virtual void SerializeTo(std::iostream *hStream);

    virtual void DeserializeFrom(std::istream *hStream);
};

/**
 WWD extended meta
 typy danych;
    zstring - oznacza string zakonczony nullem

 Ogolny schemat:
 1) tag poczatkowy
    string 9 bajtow - "<wm_meta>" (bez nulla na koncu)
 2) bloki
    kazdy blok sklada sie z naglowka bloku i nastepujacych po nim danych bloku.
    - budowa naglowka bloku
      1 bajt - identyfikator bloku [wymienione dalej]
      4 bajty - integer, dlugosc danych bloku

    Jesli typ bloku nie jest obslugiwany, wystarczy przeskoczyc do przodu o dlugosc
    danych bloku.

    Pierwszym blokiem musi byc blok MetaInfo (ID=0), a ostatnim MetaEndInfo (ID=1),
    natomiast miedzy nimi moga wystepowac w dowolnej kolejnosci dowolne bloki. Zaden
    blok nie moze sie powtarzac (tzn. nie moga istniec dwa bloki o takim samym ID).

 3) tag koncowy
    string 10 bajtow - "</wm_meta>" (bez nulla na koncu)

 typy blokow:
    MetaInfo (ID - 0)
        4 bajty - integer, dlugosc wszystkich danych (wlacznie z tagiem poczatku i konca)

    MetaEndInfo (ID - 1)
        4 bajty - integer, dlugosc wszystkich danych (wlacznie z tagiem poczatku i konca)

    Header (ID - 2)
        4 bajty - integer, build WM z ktorego zostala zapisana mapa
        zstring - wersja WM z ktorego zostala zapisana mapa (np. "0.1.2.3")
        4 bajty - integer, build mapy
        zstring - wersja mapy jako string
        zstring - opis mapy

    Guides (ID - 3)
        4 bajty - integer, ilosc linii pomocniczych
        kolejno kazda linia pomocnicza:
            1 bajt - orientacja linii (1 - pozioma, 0 - pionowa)
            4 bajty - integer, pozycja linii
**/

#endif // IO_WWDX
