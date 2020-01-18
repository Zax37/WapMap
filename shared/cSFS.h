#ifndef H_C_SFS
#define H_C_SFS
#include <vector>
#include <sys\types.h>
#include <sys\stat.h>
#include <istream>

#define SFS_BUILD 2

//configuration:
//#define SFS_COMPILER
//#define SFS_LOG

#ifdef SFS_COMPILER
class cSFS_Compiler;
#endif

class cSFS_File;

class cSFS_Member {
 private:
  int iSize;
  char * szName;
  int iOffset;

  friend class cSFS_Directory;
  friend class cSFS_File;
#ifdef SFS_COMPILER
  friend class cSFS_Compiler;
#endif
  friend class cSFS_Repository;
 public:
  virtual bool IsContainer(){ return 0; };
  int GetSize(){ return iSize; };
  const char * GetName(){ return szName; };
};

class cSFS_Directory : public cSFS_Member {
 private:
  std::vector<cSFS_Member*> vMembers;

  cSFS_Directory();
  cSFS_Directory(const char * pszName);
  void AddFile(cSFS_File * pFile);
  cSFS_Directory * CreateSubdir(const char * pszName);
  void Assign(cSFS_Member * pMember);
  void UpdateSizeRecursive();
  cSFS_Member * GetMember(const char * pszName);
  cSFS_Member * GetMember(int piID){ return vMembers[piID]; };

#ifdef SFS_COMPILER
 friend class cSFS_Compiler;
#endif
 friend class cSFS_Repository;

 public:
  #ifdef DEBUG
   void PrintStructure(bool pbRecursive = 0, int piRecursionLvl = 0);
  #endif

  bool Exists(const char * pszName);
  virtual bool IsContainer(){ return 1; };
  int GetMembersNum(){ return vMembers.size(); };
  int GetMembersNumTotal();
  int GetDirsNumTotal();
  int GetDirsNum();
  int GetFilesNum();
  int GetFilesNumTotal();
};

class cSFS_File : public cSFS_Member {
 private:
  void * hData;
#ifdef SFS_COMPILER
 friend class cSFS_Compiler;
#endif
 friend class cSFS_Repository;
 public:
  cSFS_File();
  ~cSFS_File();
  virtual bool IsContainer(){ return 0; };
};

class cSFS_Repository {
 private:
  int iOffsetCursor;
  int iFilesInRoot;
  std::istream * isSource;
  bool bFile;
  void * hData;
  //char * szFile;
  int iLength;
  int iFiles;
  int iDirs;
  cSFS_Directory Root;
  void ReadDir(cSFS_Directory * pDir, std::istream * is);
  cSFS_Member * GetMember(const char * pszFilename);
 public:
  cSFS_Repository(void * phData, int piLength);
  cSFS_Repository(const char * pszFilename);
  ~cSFS_Repository();
  int GetFilesNum();
  int GetFilesNumTotal();
  int GetDirsNum();
  int GetDirsNumTotal();
  int GetRepositorySize();
  bool IsFile(const char * pszFilename);
  bool IsDir(const char * pszFilename);
  bool Exists(const char * pszFilename);
  void * GetFileAsRawData(const char * pszFilename, int * oDataSize = NULL); //header identical to one in Manager
  cSFS_Directory * GetDir(const char * szName);
  cSFS_File * GetFile(const char * szName);
  #ifdef DEBUG
  void PrintStructure();
  #endif
};

class cSFS_Manager {
 private:
  std::vector<cSFS_Repository*> vRepositories;
 public:
  cSFS_Manager();
  ~cSFS_Manager();

  void AttachRepository(cSFS_Repository * phRepo);
  void DetachRepository(cSFS_Repository * phRepo);

  void * GetFileAsRawData(const char * pszFilename, int * oDataSize = NULL);

  #ifdef DEBUG
  void PrintStructure();
  #endif
};

#ifdef SFS_COMPILER

class cSFS_Compiler {
 private:
  cSFS_Directory Root;
  void CompileDir(cSFS_Directory * pDir, FILE * pF);
  void CompileFile(cSFS_File * pDir, FILE * pF);
 public:
  cSFS_Compiler();
  ~cSFS_Compiler();
  void CreateDir(const char * pszDirectory);
  void DeleteDir(const char * pszDirectory);
  void AddFile(const char * pszFilename, const char * pszAs);
  void CompileTo(const char * pszFilename);
  #ifdef DEBUG
  void PrintStructure();
  #endif
};

#endif

#endif

/**
SFS repository file reference

File header (24 bytes):
Format header (3 bytes): SFS
Format version (1 byte): may vary, act. 1
Files count (4 bytes, int): self explanatory
Directories count (4 bytes, int): same here, it does not include the root directory (/)
Reserved space for future features (12 bytes):

Body (varies on repository size):
Here goes the root directory.

End:
EOF sign (1 byte): always 255

Each directory and file is treaten as a "member". Member structure is static, and consist of header and body.
File whole size formula is: 70+s, where s is data size.
Directory whole size formula is: 94+72*m+s, where m is members count and s is sum of files size. Body size is 24+72*m+s.
Header (70 bytes):
Name (64 bytes, null terminated string, can include any characters except '/')
Size of body in bytes (4 bytes, int)
Type (1 byte, actually can be 0 - file, 1 - container (dir))
Reserved space (1 byte).
Body;
Vary, raw data of file or data of directory.

If member is a directory, there's directory header in the body.

Header (24 bytes):
Number of members (4 byte, int): self explanatory
Reserved space (20 bytes).

Now there's members lexicon. Each member of the directory has got its own 'line', which consists of: (73b)
Member's name (64 bytes, null terminated string, can include any characters except '/')
Member's size (4 bytes, int)
Position offset according to directory's header start (4 bytes, int)
Is member a container (1 byte)

After that, there are member structures (recursively ;p).

Example of simple repository. Note: [x] represents byte.
**/
