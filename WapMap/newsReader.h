#ifndef H_C_NEWSREADER
#define H_C_NEWSREADER

class cNewsReader {
private:
    int iNewsCountFromTCR;
    char **szNewsFromTCR;
    bool bSuccessTCR;
    bool bTCRReady, bWMReady, bNewTCR;
public:
    cNewsReader();

    ~cNewsReader();

    int GetNewsCountFromTCR() { return iNewsCountFromTCR; };

    bool ErrorLoadingTCR() { return !bSuccessTCR; };

    bool IsTCRReady() { return bTCRReady; };

    bool IsWMReady() { return bWMReady; };

    void MarkTCRReady(bool bErrors) {
        bTCRReady = 1;
        bSuccessTCR = !bErrors;
    };

    void SetTCRNewsCount(int i) {
        iNewsCountFromTCR = i;
        szNewsFromTCR = new char *[i];
    };

    void SetTCRNewsContentTo(int i, char *val) { szNewsFromTCR[i] = val; };

    const char *GetTCRNews(int i) { return szNewsFromTCR[i]; };

    bool AreNewNewsFromTCR() { return bNewTCR; };

    void MarkNewsOnTCR(bool m) { bNewTCR = m; };
};

#endif
