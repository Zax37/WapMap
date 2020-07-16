#include "cSFS.h"

#ifdef DEBUG
#include <conio.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#define SFS_NCLEANSTR(x, y) char * x = new char[(y)]; \
                            SFS_CLEANSTR(x, y)
#define SFS_CLEANSTR(x, y) for(size_t __i=0;__i<(y);__i++) \
                            x[__i] = '\0'
#define SFS_strcpy(x, y) strncpy(x, y, strlen(y))

#define SFS_readint(x, y) x->read((char*)(y), 4)
#define SFS_readbyte(x, y) x->read((char*)(y), 1)

typedef unsigned char byte;

cSFS_Directory::cSFS_Directory() {
    iSize = 0;
    szName = new char[64];
    SFS_CLEANSTR(szName, 64);
}

cSFS_Directory::cSFS_Directory(const char *pszName) {
    iSize = 0;
    szName = new char[64];
    SFS_CLEANSTR(szName, 64);
    SFS_strcpy(szName, pszName);
}

bool cSFS_Directory::Exists(const char *pszName) {
    for (size_t i = 0; i < vMembers.size(); i++) {
        if (!strcmp(pszName, vMembers[i]->GetName())) {
            return 1;
        }
    }
    return 0;
}

cSFS_Member *cSFS_Directory::GetMember(const char *pszName) {
    if (strchr(pszName, '/') != NULL) {
        char sub[64];
        for (int i = 0; i < 63; i++) {
            if (pszName[i] == '/') {
                sub[i] = '\0';
                break;
            } else
                sub[i] = pszName[i];
        }

        if (Exists(sub)) {
            SFS_NCLEANSTR(rest, strlen(pszName) - strlen(sub));
            for (size_t i = strlen(sub) + 1; i < strlen(pszName); i++) {
                rest[i - strlen(sub) - 1] = pszName[i];
            }
            rest[strlen(pszName) - strlen(sub) - 1] = '\0';
            cSFS_Member *ret = ((cSFS_Directory *) GetMember(sub))->GetMember(rest);
            delete[] rest;
            return ret;
        } else {
            return NULL;
        }
    } else {
        for (size_t i = 0; i < vMembers.size(); i++) {
            if (!strcmp(pszName, vMembers[i]->GetName())) {
                return vMembers[i];
            }
        }
    }
    return NULL;
}

void cSFS_Directory::AddFile(cSFS_File *pFile) {
    iSize += pFile->GetSize();
    vMembers.push_back(pFile);
}

int cSFS_Directory::GetDirsNum() {
    int ret = 0;
    for (size_t i = 0; i < vMembers.size(); i++)
        if (vMembers[i]->IsContainer())
            ret++;
    return ret;
}

cSFS_Directory *cSFS_Repository::GetDir(const char *szName) {
    if (!IsDir(szName))
        return NULL;
    return (cSFS_Directory *) GetMember(szName);
}

cSFS_File *cSFS_Repository::GetFile(const char *szName) {
    if (!IsFile(szName))
        return NULL;
    return (cSFS_File *) GetMember(szName);
}

cSFS_Directory *cSFS_Directory::CreateSubdir(const char *pszName) {
    cSFS_Directory *ret = NULL;
    if (strchr(pszName, '/') != NULL) {
        char sub[64];
        for (int i = 0; i < 63; i++) {
            if (pszName[i] == '/') {
                sub[i] = '\0';
                break;
            } else
                sub[i] = pszName[i];
        }
        cSFS_Directory *subdir;

        if (Exists(sub)) {
            subdir = (cSFS_Directory *) GetMember(sub);
        } else {
            subdir = new cSFS_Directory(sub);
            vMembers.push_back(subdir);
        }

        SFS_NCLEANSTR(rest, strlen(pszName) - strlen(sub));
        for (size_t i = strlen(sub) + 1; i < strlen(pszName); i++) {
            rest[i - strlen(sub) - 1] = pszName[i];
        }
        rest[strlen(pszName) - strlen(sub) - 1] = '\0';
        ret = subdir->CreateSubdir(rest);
        delete[] rest;
    } else {
        ret = new cSFS_Directory(pszName);
        vMembers.push_back(ret);
    }
    return ret;
}

int cSFS_Directory::GetMembersNumTotal() {
    int ret = 0;
    for (size_t i = 0; i < vMembers.size(); i++) {
        ret++;
        if (vMembers[i]->IsContainer()) {
            ret += ((cSFS_Directory *) vMembers[i])->GetMembersNumTotal();
        }
    }
    return ret;
}

int cSFS_Directory::GetDirsNumTotal() {
    int ret = 0;
    for (size_t i = 0; i < vMembers.size(); i++) {
        if (vMembers[i]->IsContainer()) {
            ret++;
            ret += ((cSFS_Directory *) vMembers[i])->GetDirsNumTotal();
        }
    }
    return ret;
}

#ifdef DEBUG
void cSFS_Directory::PrintStructure(bool pbRecursive, int piRecursionLvl)
{
    for (int i = 0; i < vMembers.size(); i++) {
        for (int x = 0; x < piRecursionLvl; x++)
            printf(" ");
        if (vMembers[i]->IsContainer()) {
            printf("%s (dir, %d bytes, %d files)", vMembers[i]->GetName(), vMembers[i]->GetSize(), ((cSFS_Directory*)vMembers[i])->GetMembersNum());
            if (pbRecursive && ((cSFS_Directory*)vMembers[i])->GetMembersNum() != 0) {
                printf(":\n");
                ((cSFS_Directory*)vMembers[i])->PrintStructure(1, piRecursionLvl + 1);
            }
            else
                printf("\n");
        }
        else {
            printf("%s (file, %d bytes)\n", vMembers[i]->GetName(), vMembers[i]->GetSize());
        }
    }
}
#endif

#ifdef SFS_COMPILER
cSFS_Compiler::cSFS_Compiler()
{

}

cSFS_Compiler::~cSFS_Compiler()
{

}

void cSFS_Compiler::CompileFile(cSFS_File * pFile, FILE * pF)
{
    //member header
    fwrite(pFile->szName, 64, 1, pF);
    fwrite(&pFile->iSize, 1, sizeof(int), pF);
    byte b = 0;
    fwrite(&b, 1, 1, pF);
    fwrite(&b, 1, 1, pF);
    //body
    fwrite(pFile->hData, pFile->iSize, 1, pF);
}

void cSFS_Compiler::CompileDir(cSFS_Directory * pDir, FILE * pF)
{
    int i = 0;
    byte b = 0;
    //member header
    fwrite(pDir->szName, 64, 1, pF);

    //body size
    i = pDir->GetSize() + (pDir->GetMembersNumTotal() - pDir->GetDirsNumTotal()) * 142 + pDir->GetDirsNumTotal() * 94;
    fwrite(&i, 1, sizeof(int), pF);

    //container tag
    b = 1;
    fwrite(&b, 1, 1, pF);
    b = 0;
    fwrite(&b, 1, 1, pF);

    //dir header
    i = pDir->GetMembersNum();
    fwrite(&i, 1, sizeof(int), pF);
    for (int x = 0; x < 20; x++)
        fwrite(&b, 1, 1, pF);

    //lexicon
    int offset = 0;
    for (int x = 0; x < pDir->GetMembersNum(); x++) {
        fwrite(pDir->vMembers[x]->szName, 64, 1, pF);
        i = pDir->vMembers[x]->iSize;
        fwrite(&i, 1, sizeof(int), pF);

        fwrite(&offset, 1, sizeof(int), pF);
        //printf("act %d\n", offset);
        if (pDir->vMembers[x]->IsContainer()) {
            b = 1; //((cSFS_Directory*)pDir->vMembers[x])
            //printf("offset += size %d memcount %d dircount %d\n", ((cSFS_Directory*)pDir->vMembers[x])->GetSize(), ((cSFS_Directory*)pDir->vMembers[x])->GetMembersNum(), ((cSFS_Directory*)pDir->vMembers[x])->GetDirsNumTotal());
            offset += 94 + ((cSFS_Directory*)pDir->vMembers[x])->GetSize() + (((cSFS_Directory*)pDir->vMembers[x])->GetMembersNumTotal() - ((cSFS_Directory*)pDir->vMembers[x])->GetDirsNumTotal()) * 143 + ((cSFS_Directory*)pDir->vMembers[x])->GetDirsNumTotal() * 94;
        }
        else {
            b = 0;
            //printf("offset += %d+70\n", i);
            offset += 70 + i;
        }
        fwrite(&b, 1, 1, pF);
    }
    for (int x = 0; x < pDir->GetMembersNum(); x++) {
        if (pDir->vMembers[x]->IsContainer()) {
            CompileDir((cSFS_Directory*)pDir->vMembers[x], pF);
        }
        else {
            CompileFile((cSFS_File*)pDir->vMembers[x], pF);
        }
    }
}

void cSFS_Compiler::CompileTo(const char * pszFilename)
{
    FILE * f = fopen(pszFilename, "wb");

    //header start.
    fwrite("SFS", 3, 1, f);
    byte b = 1;
    fwrite(&b, 1, 1, f);
    int i = Root.GetMembersNumTotal();
    fwrite(&i, 1, sizeof(int), f);
    i = Root.GetDirsNumTotal();
    fwrite(&i, 1, sizeof(int), f);

    b = 0;
    for (int x = 0; x < 12; x++)
        fwrite(&b, 1, 1, f);
    //header done.

    CompileDir(&Root, f);

    //eof start.
    b = 255;
    fwrite(&b, 1, 1, f);
    //eof done.

    fclose(f);
}

#ifdef DEBUG
void cSFS_Compiler::PrintStructure()
{
    printf("/ (%d dirs, %d files, %db):\n", Root.GetDirsNumTotal(), Root.GetMembersNumTotal() - Root.GetDirsNumTotal(), Root.GetSize());
    if (Root.GetMembersNum() == 0) {
        printf(" empty\n");
    }
    else {
        Root.PrintStructure(1, 1);
    }
}
#endif

void cSFS_Compiler::CreateDir(const char * pszDirectory)
{
    Root.CreateSubdir(pszDirectory);
}

void cSFS_Compiler::AddFile(const char * pszFilename, const char * pszAs)
{
    cSFS_Directory * saveTo = &Root;
    cSFS_File * ptr = new cSFS_File();

    if (strchr(pszAs, '/') != NULL) {
        SFS_NCLEANSTR(rest, strlen(pszAs) + 1);
        SFS_strcpy(rest, pszAs);

        while (strchr(rest, '/') != NULL) {
            char sub[64];
            for (int i = 0; i < 63; i++) {
                if (rest[i] == '/') {
                    sub[i] = '\0';
                    break;
                }
                else
                    sub[i] = rest[i];
            }
            saveTo = (cSFS_Directory*)saveTo->GetMember(sub);
            for (int i = strlen(sub) + 1; i < strlen(rest); i++) {
                rest[i - strlen(sub) - 1] = rest[i];
            }
            rest[strlen(rest) - strlen(sub) - 1] = '\0';
        }
        ptr->szName = new char[64];
        SFS_CLEANSTR(ptr->szName, 64);
        SFS_strcpy(ptr->szName, rest);
        delete[] rest;
    }
    else {
        ptr->szName = new char[64];
        SFS_CLEANSTR(ptr->szName, 64);
        SFS_strcpy(ptr->szName, pszAs);
    }

    struct stat fileStat;
    int err = stat(pszFilename, &fileStat);
    ptr->iSize = fileStat.st_size;

    ptr->hData = new char[ptr->iSize];
    FILE * f = fopen(pszFilename, "rb");
    fread(ptr->hData, ptr->iSize, 1, f);
    fclose(f);

    saveTo->AddFile(ptr);

    Root.UpdateSizeRecursive();
}

#endif

void cSFS_Directory::Assign(cSFS_Member *pMember) {
    vMembers.push_back(pMember);
}

void cSFS_Directory::UpdateSizeRecursive() {
    iSize = 0;
    for (size_t i = 0; i < vMembers.size(); i++) {
        if (vMembers[i]->IsContainer()) {
            ((cSFS_Directory *) vMembers[i])->UpdateSizeRecursive();
        }
        iSize += vMembers[i]->GetSize();
    }
}

cSFS_Repository::cSFS_Repository(void *phData, int piLength) {
    hData = phData;
    iLength = piLength;
    isSource = new std::istringstream(std::string((const char *) hData, iLength), std::ios::in | std::ios::binary);
    bFile = 0;
    if (isSource->fail()) {
        printf("SFS: Error opening repository from memory on addr 0x%p.", phData);
    }

    char sig[4];
    isSource->read(sig, 3);
    sig[3] = '\0';
    if (strcmp(sig, "SFS") != 0) {
#ifdef DEBUG
        printf("SFS: Repository '0x%d' seems to be invalid format.\n", phData);
#endif
    }
    char b;
    isSource->read(&b, 1);
    if (b != 1) {
#ifdef DEBUG
        printf("SFS: Repository '0x%d' version is %d, which is not supported.\n", phData, b);
#endif
    }
    SFS_readint(isSource, &iFiles);
    SFS_readint(isSource, &iDirs);
    isSource->seekg(76, std::ios_base::cur);
    int size = 0;
    SFS_readint(isSource, &size);
#ifdef LOG
    printf("SFS: Repository contains %d files and %d dirs (%d bytes).\n", iFiles, iDirs, size);
#endif
    int g = isSource->tellg();
    isSource->seekg(0, std::ios::end);
    int streamsize = isSource->tellg();
    isSource->seekg(g);
    if (streamsize < size) {
#ifdef DEBUG
        printf("SFS: Warning, repository '0x%d' may be corrupted (only %d bytes).\n", phData, streamsize);
#endif
    }
    isSource->seekg(2, std::ios_base::cur);
    int members = 0;
    SFS_readint(isSource, &members);
#ifdef SFS_LOG
    printf("SFS: Root contains %d files.\n", members);
#endif
    isSource->seekg(20, std::ios_base::cur);
    iFilesInRoot = members;
    iOffsetCursor = 118 + members * 73;
    for (int i = 0; i < members; i++) {
        SFS_NCLEANSTR(name, 64);
        int size, offset;
        byte dir;
        isSource->read(name, 64);
        SFS_readint(isSource, &size);
        SFS_readint(isSource, &offset);
        SFS_readbyte(isSource, &dir);
        iOffsetCursor = 118 + members * 73 + offset;
#ifdef SFS_LOG
        printf("SFS: Found file '%s' (%d bytes, offset %d (%d), dir %d)\n", name, size, offset, iOffsetCursor, dir);
#endif
        if (dir == '\1') {
            cSFS_Directory *ptr = Root.CreateSubdir(name);
            ptr->iSize = size;
            ptr->iOffset = iOffsetCursor;
        } else {
            cSFS_File *ptr = new cSFS_File;
            ptr->szName = new char[strlen(name) + 1];
            SFS_CLEANSTR(ptr->szName, strlen(name) + 1);
            SFS_strcpy(ptr->szName, name);
            ptr->iOffset = iOffsetCursor;
            ptr->iSize = size;
            Root.AddFile(ptr);
        }
        delete[] name;
    }
    for (int i = 0; i < Root.GetMembersNum(); i++) {
        if (Root.GetMember(i)->IsContainer()) {
            ReadDir((cSFS_Directory *) Root.GetMember(i), isSource);
        }
    }
}

cSFS_Repository::cSFS_Repository(const char *pszFilename) {
    hData = NULL;
    iOffsetCursor = 0;

    isSource = new std::ifstream(pszFilename, std::ios::in | std::ios::binary);
    bFile = 1;
    if (isSource->fail()) {
        printf("SFS: Error opening '%s'.", pszFilename);
    }

    char sig[4];
    isSource->read(sig, 3);
    sig[3] = '\0';
    if (strcmp(sig, "SFS") != 0) {
#ifdef DEBUG
        printf("SFS: Repository '%s' seems to be invalid format.\n", pszFilename);
#endif
    }
    char b;
    isSource->read(&b, 1);
    if (b != 1) {
#ifdef DEBUG
        printf("SFS: Repository '%s' version is %d, which is not supported.\n", pszFilename, b);
#endif
    }
    SFS_readint(isSource, &iFiles);
    SFS_readint(isSource, &iDirs);
    isSource->seekg(76, std::ios_base::cur);
    int size = 0;
    SFS_readint(isSource, &size);
#ifdef LOG
    printf("SFS: Repository contains %d files and %d dirs (%d bytes).\n", iFiles, iDirs, size);
#endif
    struct stat fileStat;
    stat(pszFilename, &fileStat);
    if (fileStat.st_size < size) {
#ifdef DEBUG
        printf("SFS: Warning, repository '%s' may be corrupted (only %d bytes).\n", pszFilename, fileStat.st_size);
#endif
    }
    isSource->seekg(2, std::ios_base::cur);
    int members = 0;
    SFS_readint(isSource, &members);
#ifdef SFS_LOG
    printf("SFS: Root contains %d files.\n", members);
#endif
    isSource->seekg(20, std::ios_base::cur);
    iFilesInRoot = members;
    iOffsetCursor = 118 + members * 73;
    for (int i = 0; i < members; i++) {
        SFS_NCLEANSTR(name, 64);
        int size, offset;
        byte dir;
        isSource->read(name, 64);
        SFS_readint(isSource, &size);
        SFS_readint(isSource, &offset);
        SFS_readbyte(isSource, &dir);
        iOffsetCursor = 118 + members * 73 + offset;
#ifdef SFS_LOG
        printf("SFS: Found file '%s' (%d bytes, offset %d (%d), dir %d)\n", name, size, offset, iOffsetCursor, dir);
#endif
        if (dir == '\1') {
            cSFS_Directory *ptr = Root.CreateSubdir(name);
            ptr->iSize = size;
            ptr->iOffset = iOffsetCursor;
        } else {
            cSFS_File *ptr = new cSFS_File;
            ptr->szName = new char[strlen(name) + 1];
            SFS_CLEANSTR(ptr->szName, strlen(name) + 1);
            SFS_strcpy(ptr->szName, name);
            ptr->iOffset = iOffsetCursor;
            ptr->iSize = size;
            Root.AddFile(ptr);
        }
        delete[] name;
    }
    for (int i = 0; i < Root.GetMembersNum(); i++) {
        if (Root.GetMember(i)->IsContainer()) {
            ReadDir((cSFS_Directory *) Root.GetMember(i), isSource);
        }
    }
}

cSFS_Repository::~cSFS_Repository() {
    if (bFile) {
        ((std::ifstream *) isSource)->close();
    } else {
        delete[](unsigned char *) hData;
    }
}

int cSFS_Repository::GetFilesNum() {
    return Root.GetMembersNum() - Root.GetDirsNum();
}

int cSFS_Repository::GetFilesNumTotal() {
    return Root.GetMembersNumTotal() - Root.GetDirsNumTotal();
}

int cSFS_Repository::GetDirsNum() {
    return Root.GetDirsNum();
}

int cSFS_Repository::GetDirsNumTotal() {
    return Root.GetDirsNum();
}

bool cSFS_Repository::IsFile(const char *pszFilename) {
    cSFS_Member *mem = GetMember(pszFilename);
    if (mem == NULL)
        return 0;
    if (mem->IsContainer())
        return 0;
    return 1;
}

bool cSFS_Repository::IsDir(const char *pszFilename) {
    cSFS_Member *mem = GetMember(pszFilename);
    if (mem == NULL)
        return 0;
    if (mem->IsContainer())
        return 1;
    return 0;
}

bool cSFS_Repository::Exists(const char *pszFilename) {
    cSFS_Member *mem = GetMember(pszFilename);
    if (mem == NULL)
        return 0;
    return 1;
}

char *cSFS_Repository::GetFileAsRawData(const char *pszFilename, int *oDataSize) {
    if (!IsFile(pszFilename)) {
        if (oDataSize != NULL)
            *oDataSize = 0;
        return NULL;
    }
    cSFS_File *file = (cSFS_File *) GetMember(pszFilename);
    isSource->seekg(file->iOffset + 70);
    char *data = new char[file->iSize];
    //printf("reading. seek'd to %d, reading %d bytes.\n", file->iOffset, file->iSize);
    isSource->read((char *) data, file->iSize);
    if (oDataSize != NULL)
        *oDataSize = file->iSize;
    return data;
}

cSFS_Member *cSFS_Repository::GetMember(const char *pszFilename) {
    return Root.GetMember(pszFilename);
}

int cSFS_Repository::GetRepositorySize() {
    int ret = 0;
    for (int i = 0; i < Root.GetMembersNum(); i++) {
        ret += Root.vMembers[i]->GetSize();
    }
    return ret;
}

void cSFS_Repository::ReadDir(cSFS_Directory *pDir, std::istream *is) {
    int cur = is->tellg();
    is->seekg(70 + pDir->iOffset, std::ios_base::beg);
    int imemnum = 0;
    SFS_readint(is, &imemnum);
#ifdef SFS_LOG
    printf("SFS: Found %d files in dir %s.\n", imemnum, pDir->szName);
#endif
    is->seekg(20, std::ios_base::cur);
    //pDir->iOffset = ftell(pF);
    for (int i = 0; i < imemnum; i++) {
        is->seekg(72, std::ios_base::cur);
        byte b = 0;
        SFS_readbyte(is, &b);

        cSFS_Member *ptr;

        if (b) {
            ptr = new cSFS_Directory();
        } else {
            ptr = new cSFS_File();
        }
        is->seekg(-73, std::ios_base::cur);
        ptr->szName = new char[64];
        is->read(ptr->szName, 64);
        SFS_readint(is, &ptr->iSize);
        SFS_readint(is, &ptr->iOffset);
        //printf("%d += %d + 94 + %d*73\n", ptr->iOffset, pDir->iOffset, imemnum);
        ptr->iOffset += pDir->iOffset + 94 + imemnum * 73;
        if (b) {
#ifdef SFS_LOG
            printf("SFS: Found dir '%s', size %d, offset %d\n", ptr->szName, ptr->iSize, ptr->iOffset);
#endif
            ReadDir((cSFS_Directory *) ptr, is);
        } else {
#ifdef SFS_LOG
            printf("SFS: Found file '%s', size %d, offset %d\n", ptr->szName, ptr->iSize, ptr->iOffset);
#endif
        }

        pDir->Assign(ptr);
        is->seekg(1, std::ios_base::cur);
    }
    is->seekg(cur);
}

void cSFS_Repository::Unpack(cSFS_Directory *dir) {
    if (dir == NULL) dir = &Root;

    for (auto member : dir->vMembers) {
        if (member->IsContainer()) {
            Unpack((cSFS_Directory *) member);
        } else {
            char *buffer = new char[member->iSize];
            isSource->seekg(member->iOffset + 70);
            isSource->read(buffer, member->iSize);
            std::ofstream f(std::string("res/") + member->szName, std::ofstream::binary);
            f.write(buffer, member->iSize);
            f.close();
        }
    }
}

#ifdef DEBUG
void cSFS_Repository::PrintStructure()
{
    printf("/ (%d dirs, %d files, %db):\n", Root.GetDirsNumTotal(), Root.GetMembersNum() - Root.GetDirsNumTotal(), Root.GetSize());
    if (Root.GetMembersNum() == 0) {
        printf(" empty\n");
    }
    else {
        Root.PrintStructure(1, 1);
    }
}
#endif

cSFS_File::cSFS_File() {
    iSize = 0;
    szName = new char[64];
    SFS_CLEANSTR(szName, 64);
    hData = NULL;
}

cSFS_File::~cSFS_File() {
    delete[] szName;
    if (hData != NULL)
        delete[](unsigned char *) hData;
}
