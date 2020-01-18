#include "cREZ.h"

#include <conio.h>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "commonFunc.h"

bool REZ::ElementComparison(REZ::Element * one, REZ::Element * two)
{
    return SHR::CompareStrings(one->GetName(), two->GetName());
}

bool REZ::ElementComparisonInt(REZ::Element * one, REZ::Element * two)
{
    return SHR::CompareStringsIntSupport(one->GetName(), two->GetName());
}

REZ::File::File()
{
   // printf("constructor file\n");
}

REZ::Dir::Dir()
{
    //printf("constructor dir\n");
}

REZ::Dir::~Dir()
{
    Reset();
    delete [] m_szName;
}

void REZ::Dir::Reset()
{
    for(size_t i=0;i<m_vElements.size();i++)
     delete m_vElements[i];
    m_vElements.clear();
}

REZ::File::~File()
{
    delete [] m_szName;
}

REZ::Parser::Parser()
{
    m_hRoot.m_uiOffset = 0;
    m_hRoot.m_uiLen = 0;
    m_hRoot.m_szName = '\0';
    m_hRoot.m_hOwner = this;
    m_bInited = 0;
    m_bFromFile = 0;
    m_isSource = 0;
    iLastRead = 0;
}

REZ::Parser::Parser(const char * pszFile)
{
    m_hRoot.m_uiOffset = 0;
    m_hRoot.m_uiLen = 0;
    m_hRoot.m_szName = '\0';
    m_hRoot.m_hOwner = this;
    m_bInited = 0;
    m_bFromFile = 1;
    m_isSource = 0;
    LoadFile(pszFile);
}

REZ::Parser::~Parser()
{
    if( m_isSource != 0 ){
     ((std::ifstream*)m_isSource)->close();
     delete m_isSource;
    }
}

void * REZ::File::GetData(int * oiDataSize)
{
    std::istream * stream = m_hOwner->GetSourceStream();
    unsigned int pos = stream->tellg();

    stream->seekg(m_uiOffset);
    void * data = new char[m_uiLen];
    *oiDataSize = m_uiLen;
    stream->LLEN(data, m_uiLen);

    stream->seekg(pos);
    return data;
}

void REZ::Parser::LoadFile(const char * pszFile)
{
    m_bFromFile = 1;
    strFilePath = std::string(pszFile);
    AttachFile(1);
}

std::istream * REZ::Parser::GetSourceStream()
{
    if( !m_bInited ){
     throw REZ_EXCEPTION("Trying to load from undefined parser.", REZERR_INTERNAL_UNSET_PARSER);
    }
    return m_isSource;
}

void REZ::Parser::ReadDir(Dir * phDir, int piRLevel)
{
    unsigned int uiSeekTo = m_isSource->tellg();

    m_isSource->seekg(phDir->m_uiOffset);

    unsigned int uiBytesRead = 0;
    while( uiBytesRead < phDir->m_uiLen-1 ){
      //for(int f=0;f<piRLevel;f++)
       //printf(" ");
     //printf("%d/%d &%d\n", uiBytesRead, phDir->m_uiLen, (int)m_isSource->tellg());
     byte b;
     m_isSource->LBYTE(b);
     m_isSource->seekg(3, std::ios_base::cur);
     uiBytesRead += 4;
     Element * ptr = NULL;
     //printf("step0\n");
     //try{
      if( b ){
       ptr = new Dir();
      }else{
       ptr = new File();
      }
     /*} catch(...){
      printf("BAD_ALLOC\n");
     }*/
     //if( ptr == NULL )
     // printf("ERROR W CHUJ\n");
     //printf("step 0.5\n");
     ptr->m_hOwner = phDir->m_hOwner;
     m_isSource->LLEN(&ptr->m_uiOffset, 4);
     m_isSource->LLEN(&ptr->m_uiLen, 4);
     m_isSource->LLEN(&ptr->m_uiTime, 4);
     uiBytesRead += 12;
     //printf("step1\n");
     if( b ){ //dir
      char name[128];
      for(int i=0;i<128;i++){
       m_isSource->LBYTE(name[i]);
       uiBytesRead++;
       if( name[i] == '\0' )
        break;
      }
      //ptr->m_szName = new char[strlen(name)+1];
      ptr->m_szName = SHR::ToLower(name);
      //strcpy(ptr->m_szName, name);
      //printf("found dir: %s\n", name);
     //getch();
      //printf("step2\n");
      if( ptr->m_uiLen > 0 /*&& piRLevel < 2*/ ){
       ReadDir((Dir*)ptr, piRLevel+1);
      }
     }else{ //file
      m_isSource->LLEN(&(((File*)ptr)->m_uiID), 4);
      uiBytesRead += 4;
      char ext2[5];
      for(int i=0;i<4;i++){
       m_isSource->LBYTE(ext2[i]);
       uiBytesRead++;
      }
      ext2[4] = '\0';

      char ext[5];
      for(int i=0;i<5;i++)
       ext[i] = '\0';
      int cur = 0;
      for(int i=3;i>=0;i--){
       if( ext2[i] == '\0' && !cur )
        continue;
       ext[cur] = ext2[i];
       cur++;
      }
      //printf("step3\n");

      m_isSource->seekg(4, std::ios_base::cur);
      uiBytesRead += 4;
      char name[128];
      for(int i=0;i<128;i++){
       m_isSource->LBYTE(name[i]);
       uiBytesRead++;
       if( name[i] == '\0' && i > 0 && name[i-1] == '\0' )
        break;
      }
      char * tmpx = 0;
      if( ext[0] != '.' )
       tmpx = new char[strlen(name)+strlen(ext)+2];
      else
       tmpx = new char[strlen(name)+strlen(ext)+1];
      strcpy(tmpx, name);
      if( ext[0] != '.' )
       strcat(tmpx, ".");
      strcat(tmpx, ext);
      ptr->m_szName = SHR::ToLower(tmpx);
      delete [] tmpx;
     //
     //printf("read file: %s\n", name);
     //getch();
     }
     phDir->m_vElements.push_back(ptr);
     //printf("done\n");
    }
    /*byte b;
    isSource->LBYTE(b);
    if( b == 0 )
     throw REZ_EXCEPTION("Internal reading error.", REZERR_INTERNAL_READ_ERR);
    isSource->seekg(3, std::ios_base::cur);

    isSource->LLEN(&phDir->m_iuOffset);
    isSource->LLEN(&phDir->m_iuLen);
    isSource->LLEN(&phDir->m_iuTime);
    char name[128];
    for(int i=0;i<128;i++){
     isSource->LBYTE(name[i]);
     if( name[i] == '\0' )
      break;
    }
    phDir->m_szName = new char[strlen(name)+1];
    strcpy(phDir->m_szName, name);*/

    m_isSource->seekg(uiSeekTo);

}

void REZ::Parser::Reload()
{
    DetachFile();
    AttachFile(1);
}

bool REZ::Parser::AttachFile(bool bForceReload)
{
    if( m_isSource != 0 )
     return 0;

    m_isSource = new std::ifstream(strFilePath.c_str(), std::ifstream::binary | std::ifstream::in);

    unsigned int iNewRead = GetActualModTime();


    bool b = 0;
    if( (iLastRead != 0 && iLastRead != iNewRead) || bForceReload ){
     m_hRoot.Reset();
     Parse();
     b = 1;
    }
    iLastRead = iNewRead;
    return b;
}

unsigned int REZ::Parser::GetActualModTime()
{
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData = {0};
    GetFileAttributesEx(strFilePath.c_str(), GetFileExInfoStandard, &fileAttrData);
    return SHR::WinFileTimeToUnix(fileAttrData.ftLastWriteTime);
}

void REZ::Parser::DetachFile()
{
    if( m_isSource != 0 ){
     ((std::ifstream*)m_isSource)->close();
     delete m_isSource;
     m_isSource = 0;
    }
}

void REZ::Parser::Parse()
{
    byte b, b2;
    m_isSource->LBYTE(b);
    m_isSource->LBYTE(b2);
    if( b != 0x0D || b2 != 0x0A )
     throw REZ_EXCEPTION("Invalid format.", REZERR_INVALID_SIG);
    m_isSource->LLEN(&m_szEditor, 60);
    m_isSource->seekg(2, std::ios_base::cur);
    m_isSource->LLEN(&m_szComment, 60);
    m_isSource->seekg(3, std::ios_base::cur);

    m_isSource->LLEN(&m_iVersion, 4);
    if( m_iVersion != 1 )
     throw REZ_EXCEPTION("Unsupported version.", REZERR_UNSUPPORTED_VERSION);

    m_isSource->LLEN(&m_hRoot.m_uiOffset, 4);
    m_isSource->LLEN(&m_hRoot.m_uiLen, 4);

    if( m_hRoot.m_uiLen != 0 )
     ReadDir(&m_hRoot);

    m_bInited = 1;
}

int REZ::Dir::GetFilesCount()
{
    int ret = 0;
    for(int i=0;i<GetElementsCount();i++){
     if( m_vElements[i]->IsFile() ){
      ret++;
     }
    }
    return ret;
}

int REZ::Dir::GetDirsCount()
{
    int ret = 0;
    for(int i=0;i<GetElementsCount();i++){
     if( m_vElements[i]->IsDir() ){
      ret++;
     }
    }
    return ret;
}

int REZ::Dir::GetElementsCountR()
{
    int ret = 0;
    for(int i=0;i<GetElementsCount();i++){
     if( m_vElements[i]->IsDir() ){
      ret += ((Dir*)m_vElements[i])->GetElementsCountR();
     }
     ret++;
    }
    return ret;
}

int REZ::Dir::GetFilesCountR()
{
    int ret = 0;
    for(int i=0;i<GetElementsCount();i++)
     if( m_vElements[i]->IsDir() )
      ret += ((Dir*)m_vElements[i])->GetFilesCountR();
     else
      ret++;
    return ret;
}

int REZ::Dir::GetDirsCountR()
{
    int ret = 0;
    for(int i=0;i<GetElementsCount();i++)
     if( m_vElements[i]->IsDir() ){
      ret += ((Dir*)m_vElements[i])->GetDirsCountR();
      ret++;
     }
    return ret;
}

REZ::Element * REZ::Dir::GetElement(const char * pszName)
{
    char * buf = 0;
    const char * filepath = pszName;
    if( strchr(pszName, '\\') != NULL ){
     char search[2] = {'\\', '\0'};
     char repfor[2] = {'/', '\0'};
     buf = SHR::Replace(pszName, search, repfor);
     filepath = buf;
    }
    if( filepath[0] == '/' ){
     char * buf2 = new char[strlen(filepath)];
     for(size_t i=0;i<strlen(filepath)-1;i++)
      buf2[i] = filepath[i+1];
     buf2[strlen(filepath)-1] = '\0';
     delete [] buf;
     buf = buf2;
     filepath = buf2;
    }
    if( strchr(filepath, '/') != NULL ){
     char sub[256];
     for(int i=0;i<256;i++){
      if( filepath[i] == '/' ){
       sub[i] = '\0';
       break;
      }else
       sub[i] = filepath[i];
     }

     if( GetElement(sub) != NULL ){
      char * rest = new char[strlen(filepath)-strlen(sub)];
      for(size_t i=strlen(sub)+1;i<strlen(filepath);i++){
       rest[i-strlen(sub)-1] = filepath[i];
      }
      rest[strlen(filepath)-strlen(sub)-1] = '\0';
      Element * ret = ((Dir*)GetElement(sub))->GetElement(rest);
      delete [] rest;
      if( buf != 0 ) delete [] buf;
      return ret;
     }else{
      if( buf != 0 )
       delete [] buf;
      return NULL;
     }
    }else{
     for(size_t i=0;i<m_vElements.size();i++){
      if( !stricmp(filepath, m_vElements[i]->GetName()) ){
       if( buf != 0 )
        delete [] buf;
       return m_vElements[i];
      }
     }
    }
    if( buf != 0 )
     delete [] buf;
    return NULL;
}

void * REZ::File::GetDataChunk(int iSize)
{
    std::istream * stream = m_hOwner->GetSourceStream();
    unsigned int pos = stream->tellg();

    stream->seekg(m_uiOffset);
    if( !stream->good() )
     printf("STREAM BAD!!!");
    void * data = new unsigned char[iSize];
    stream->LLEN(data, iSize);

    stream->seekg(pos);
    return data;
}

void REZ::Dir::Sort(REZ_SORTMETHOD pMethod)
{
    if( pMethod == Sort_Default )
     std::sort(m_vElements.begin(), m_vElements.end(), ElementComparison);
    else if( pMethod == Sort_ByInts )
     std::sort(m_vElements.begin(), m_vElements.end(), ElementComparisonInt);
}
