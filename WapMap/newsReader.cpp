#include "newsReader.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include "../shared/commonFunc.h"
#include "globals.h"
#include "version.h"
#include <process.h>

static size_t MsgCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    fwrite(ptr, size, nmemb, (FILE *) data);
    return (size_t) (size * nmemb);
}

void _ThreadedLoadRSS_TCR(void *pParams) {
    cNewsReader *owner = (cNewsReader *) pParams;
    FILE *tcrSaveFile = fopen("various/tcr.tmp", "w");

    CURL *curlTCR = curl_easy_init();
    char adress[512];
    sprintf(adress, "http://%s/index.html", GV->szUrlTCR);
    curl_easy_setopt(curlTCR, CURLOPT_URL, adress);

    char client[25];
    sprintf(client, "WapMap/%d", WA_VER);
    curl_easy_setopt(curlTCR, CURLOPT_USERAGENT, client);

    curl_easy_setopt(curlTCR, CURLOPT_WRITEDATA, tcrSaveFile);
    curl_easy_setopt(curlTCR, CURLOPT_WRITEFUNCTION, MsgCallback);

    CURLM *curlMultiHandle = curl_multi_init();
    curl_multi_add_handle(curlMultiHandle, curlTCR);

    long tm;
    curl_multi_timeout(curlMultiHandle, &tm);
    if (tm > 0)
        Sleep(tm);

    while (1) {
        int alive;
        curl_multi_perform(curlMultiHandle, &alive);
        if (!alive) {
            break;
        }
    }

    curl_multi_remove_handle(curlMultiHandle, curlTCR);
    curl_easy_cleanup(curlTCR);
    curl_multi_cleanup(curlMultiHandle);
    fclose(tcrSaveFile);

    FILE *cache = fopen("various/tcr.cache", "r");
    char *cachestr = NULL;
    int cachestrlen = 0;
    if (cache != NULL) {
        fseek(cache, 0, SEEK_END);
        cachestrlen = ftell(cache);
        if (cachestrlen != 0) {
            cachestr = new char[cachestrlen + 1];
            cachestr[cachestrlen] = '\0';
            fread(&cachestr, 1, cachestrlen, cache);
        }
        fclose(cache);
    }

    char *source = NULL;
    char *site = NULL;
    int sitelen = 0;

    FILE *read = fopen("various/tcr.tmp", "rb");
    bool loadcache = 1;
    if (read != NULL) {
        fseek(read, 0, SEEK_END);
        sitelen = ftell(read);
        fseek(read, 0, SEEK_SET);
        if (sitelen != 0) {
            site = new char[sitelen + 1];
            site[sitelen] = '\0';
            fread(site, 1, sitelen, read);
            fclose(read);
            source = site;
            unlink("various/tcr.cache");
            rename("various/tcr.tmp", "various/tcr.cache");
            loadcache = 0;
        }
    }
    if (loadcache) {
        if (cachestr != NULL)
            source = cachestr;
        unlink("various/tcr.tmp");
    }

    if (cachestr == NULL || cachestr[0] == '\0') {
        owner->MarkNewsOnTCR(1);
    } else if (site == 0 || site[0] == '\0') {
        owner->MarkNewsOnTCR(0);
    } else if (cachestr != NULL) {
        bool equal = 0;
        if (cachestrlen == sitelen) {
            equal = 1;
            for (int i = 0; i < cachestrlen; i++)
                if (cachestr[i] != site[i]) {
                    equal = 0;
                    break;
                }
        }
        owner->MarkNewsOnTCR(equal);
    }

    if (source != NULL) {
        char *bar = strstr(source, "<br><hr><br>");
        bar += 12;
        char *bar2 = strstr(source, "<p><center><a href=\"archive.html\">>>");

        if (bar == NULL || bar2 == NULL) {
            owner->MarkTCRReady(1);
            return;
        }
        char *news = new char[bar2 - bar + 1];
        for (int i = 0; i < bar2 - bar; i++) {
            news[i] = bar[i];
        }
        news[bar2 - bar] = '\0';
        //printf("%s", news);

        int newscount = 0;
        char *pch = strstr(news, "<p><b>");
        if (pch != NULL)
            pch += 6;
        while (pch != NULL) {
            newscount++;
            pch = strstr(pch, "<p><b>");
            if (pch != NULL)
                pch += 6;
        }
        if (newscount == 0) {
            owner->MarkTCRReady(0);
            return;
        }
        if (newscount > 10)
            newscount = 10;

        owner->SetTCRNewsCount(newscount);

        char *it = strstr(news, "<p><b>");
        for (int i = 0; i < newscount; i++) {
            it = strstr(it, "<p><b>") + 6;
            char *newsbegin = it;

            char *newsend = strstr(it, "<p><b>");
            if (newsend == NULL) {
                newsend = news + strlen(news);
            }
            delete[] news;

            char *temporarynews = new char[newsend - newsbegin + 1];
            for (int y = 0; y < newsend - newsbegin; y++)
                temporarynews[y] = newsbegin[y];
            temporarynews[newsend - newsbegin] = '\0';
            char *tmp1 = SHR::Replace(temporarynews, "</b>", "~l~");
            delete[] temporarynews;
            char *tmp2 = SHR::Replace(tmp1, "<p>", "");
            delete[] tmp1;
            tmp1 = SHR::Replace(tmp2, "</p>", "\0");
            delete[] tmp2;
            tmp2 = SHR::Replace(tmp1, "%", "#");
            delete[] tmp1;
            tmp1 = SHR::Replace(tmp2, "\r\n", "\n");
            delete[] tmp2;

            while (1) {
                bool stop = 0;
                for (int i = 0; i < strlen(tmp1); i++) {
                    if (tmp1[i] == '<') {
                        int len = 0;
                        for (int x = i; x < strlen(tmp1); x++) {
                            if (tmp1[x] == '>') {
                                len = x - i + 1;
                                break;
                            }
                        }
                        /*printf("tag: '");
                        for(int x=i;x<len+i;x++)
                        printf("%c", tmp1[x]);
                        printf("'\n");*/
                        char *old = tmp1;
                        tmp1 = new char[strlen(tmp1) + 1 - len];
                        strncpy(tmp1, old, i + 3);
                        //printf("rest: %s\n", old+i+len);
                        strncpy(tmp1 + i, old + i + len, strlen(old) - len - i + 1);
                        delete[] old;
                    }
                    if (i == strlen(tmp1) - 1)
                        stop = 1;
                }
                if (stop)
                    break;
            }

            char *ready = new char[strlen(tmp1) + 4];
            sprintf(ready, "~y~%s", tmp1);
            ready[strlen(tmp1)] = '\0';
            delete[] tmp1;
            owner->SetTCRNewsContentTo(i, ready);
        }
    }
    if (cachestr != NULL)
        delete[] cachestr;
    if (site != NULL)
        delete[] site;

    owner->MarkTCRReady(0);
}

cNewsReader::cNewsReader() {
    bSuccessTCR = bTCRReady = bWMReady = 0;
    iNewsCountFromTCR = 0;

    _beginthread(_ThreadedLoadRSS_TCR, 0, this);
}

cNewsReader::~cNewsReader() {

}
