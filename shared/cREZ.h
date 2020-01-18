#ifndef REZ_HEADER
#define REZ_HEADER
#define REZ_V 1

#define REZ_EXCEPTION(mess, errorc) REZ::Exception(mess, \
                                         errorc, \
                                         __FILE__, \
                                         __LINE__)
#define LBYTE(b) read((char*)(&b), 1)
#define LLEN(data, len) read((char*)(data), len)

#define WBYTE(b) write((char*)(&b), 1)
#define WLEN(data, len) write((char*)(data), len)

#define REZERR_OPEN_REZ 1
#define REZERR_INVALID_SIG 2
#define REZERR_UNSUPPORTED_VERSION 3
#define REZERR_INTERNAL_READ_ERR 4
#define REZERR_INTERNAL_UNSET_PARSER 5

#ifndef byte
typedef unsigned char byte;
#endif

#include <cstring>
#include <cstdio>
#include <vector>
#include <istream>

namespace REZ {

 class Parser;

 enum REZ_SORTMETHOD {
  Sort_ByInts = 0,
  Sort_Default = 1
 };

 class Exception {
  public:
   char ErrorStr[128];
   int ErrorCode;
   char * File;
   int Line;
   void Printf(){ printf("REZ err #%d: %s on line %d in file %s.", ErrorCode, ErrorStr, Line, File); };
   Exception(char * pszErrorStr, int errorc, char * pszFile, int piLine){ strcpy(ErrorStr, pszErrorStr); Line = piLine; ErrorCode = errorc; File = new char[strlen(pszFile)+1]; strcpy(File, pszFile); };
   ~Exception(){ if( File != NULL ) delete [] File; }
 };

 class Element {
  private:
   Parser * m_hOwner;
   unsigned int m_uiOffset;
   unsigned int m_uiLen;
   unsigned int m_uiTime;
   char * m_szName;
   friend class File;
   friend class Dir;
   friend class Parser;
  public:

   virtual ~Element(){};
   unsigned int GetModificationDate(){ return m_uiTime; };
   unsigned int GetSize(){ return m_uiLen; };
   unsigned int GetOffset(){ return m_uiOffset; };
   const char * GetName(){ return (const char*)m_szName; };
   virtual bool IsDir(){ return 0; };
   virtual bool IsFile(){ return 0; };
 };

 bool ElementComparison(REZ::Element * one, REZ::Element * two);
 bool ElementComparisonInt(REZ::Element * one, REZ::Element * two);

 class File : public Element {
  private:
   unsigned int m_uiID;
   friend class Parser;
  public:
   File();
   ~File();
   unsigned int GetID(){ return m_uiID; };
   virtual bool IsDir(){ return 0; };
   virtual bool IsFile(){ return 1; };
   void * GetData(int * oiDataSize);
   void * GetDataChunk(int iSize);
 };

 class Dir : public Element {
  private:
   std::vector<Element*> m_vElements;
   friend class Parser;
   void Reset();
  public:
   Dir();
   ~Dir();
   int GetFilesCount();
   int GetDirsCount();
   int GetElementsCountR();
   int GetFilesCountR();
   int GetDirsCountR();
   int GetElementsCount(){ return m_vElements.size(); };
   Element * GetElement(int piPos){ return m_vElements[piPos]; };
   Element * GetElement(const char * pszName);
   virtual bool IsDir(){ return 1; };
   virtual bool IsFile(){ return 0; };

   void Sort(REZ_SORTMETHOD pMethod = Sort_Default);

   bool FileExists(const char * pszPath);
 };

 class Parser {
  private:
   Dir m_hRoot;
   std::istream * m_isSource;
   bool m_bFromFile, m_bInited;
   char m_szEditor[60], m_szComment[60];
   int m_iVersion;
   void Parse();
   void ReadDir(Dir * phDir, int piRLevel = 0);
   std::istream * GetSourceStream();
   std::string strFilePath;
   unsigned int iLastRead;
  friend class File;
  public:
   Parser();
   Parser(const char * pszFile);
   void LoadFile(const char * pszFile);
   ~Parser();
   bool IsFromFile(){ return m_bFromFile; };
   int GetVersion(){ return m_iVersion; };
   Dir * GetRoot(){ return &m_hRoot; };
   const char * GetEditorName(){ return (const char*)m_szEditor; };
   const char * GetComment(){ return (const char*)m_szComment; };

   void Reload();
   bool AttachFile(bool bForceReload = 0);
   void DetachFile();
   bool IsFileAttached(){ return m_isSource != 0; };

   unsigned int GetActualModTime();
   unsigned int GetReadModTime(){ return iLastRead; };
 };
};

#endif

/**
0b: 0x0D 0x0A
2b: naglowek edytora [60b]
62b: 0x0D 0x0A
64b:  naglowek ? [60b]
124b: 0x0D 0x0A 0x1A
127b: wersja rez - 1 [4b]
131b: dir offset [4b]
135b: dir size [4b]
139b: ? [4b]
143b: offset to last dir [4b]
147b: ? [4b]
151b: ? [4b]
155b: ? - 19 | 13 [4b]
159b: ? - 24 | 10[4b]
163b: ? [4b]
167b: ? - 1 [1b]
168b: dane plikow

184b+x: na kazdy element:
0b: typ (plik - 0, dir - 1) [4b]
jesli dir:
 4b: dir offset [4b]
 8b: dir len [4b]
 12b: czas [4b]
 16b: nazwa (null terminated)
jesli plik:
 4b: file offset [4b]
 8b: file len [4b]
 12b: czas [4b]
 16b: id? [4b]
 20b: odwrocone rozszerzenie [4b]
 24b: ? [4b]
 28b: nazwa (2 null terminated)
**/
