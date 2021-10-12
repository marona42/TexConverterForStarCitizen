#include "DirectXTex.h"
#include "DDS.h"
#include <stdio.h>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;
bool wflag = false, vflag = false, rflag = false, cflag = false, iflag = false; 
enum ddsStatus {none, single, mipmapped};
union dwtoch { DWORD dw; char str[4]; };
void dwtostr(DWORD dw, char *buf)
{
    dwtoch mgword;
    mgword.dw = dw;
    snprintf(buf, 5, "%s", mgword.str);
}
class ddsProject
{
    //static std::vector<ddsProject> toSingleList, toMipList;
private:
    fs::path filepath_;
    FILE* fpHeader_, *fpData_;
    ddsStatus status_;
    DirectX::DDS_HEADER hdst_;
    DirectX::DDS_HEADER_DXT10 exhdst_;
    //TODO: add dds file props
    int width_, height_, frag_;
    bool hasAlpha_;
    bool chkFrag(int fragnum)
    {
        wchar_t res[] = L".0";  res[1] += fragnum;
        fs::path tmp = res;
        return fs::exists(filepath_ / tmp);
    }
    bool chkAlpha()
    {
        return hasAlpha_ = fs::exists(filepath_ / ".a");
    }
    bool readHeader()
    {
        DWORD data;
        char buf[9] = { 0 };
        fpHeader_ = _wfopen(filepath_.wstring().c_str() , L"rb");
        if (!fpHeader_) { printf("CANNOT READ FILE %ls\n",filepath_.filename().wstring().c_str()); return false; }

        fread(&data, sizeof(DWORD), 1, fpHeader_);
        if (data != DirectX::DDS_MAGIC) { printf("NOT A DDS FILEHEADER\n"); fclose(fpHeader_);  return false; }     //read MagicWord DDS - DWORD TYPE(4byte)

        fread(&hdst_, sizeof(DirectX::DDS_HEADER), 1, fpHeader_);
        if (hdst_.dwSize != 124) { printf("CORRUPTED DDS FILEHEADER\n"); fclose(fpHeader_);  return false; }     //read size - DWORD TYPE(4byte)

        if (hdst_.ddspf.dwFlags & 0x4 && hdst_.ddspf.dwFourCC == strtol("DX10", NULL, 16))
        {
            fread(&exhdst_, sizeof(DirectX::DDS_HEADER_DXT10), 1, fpHeader_);
            vflag ? printf("DXT10 header detected.\n") : 0;
        }
        
        height_ = hdst_.dwHeight;    width_ = hdst_.dwWidth;
        frag_ = hdst_.dwMipMapCount >= 3 ? hdst_.dwMipMapCount - 3 : 0;
        if (frag_ > 0) status_ = mipmapped;
        if (iflag)
        {
            printf("Width  : %lu\n", hdst_.dwWidth);
            printf("Height : %lu\n", hdst_.dwHeight);
            printf("MipMapCount : % lu\n", hdst_.dwMipMapCount);
            hdst_.dwFlags& 0x8 || hdst_.dwFlags & 0x80000 ? printf("%s : % lu\n", hdst_.dwFlags & 0x80000?"LinearSize":"Pitch", hdst_.dwPitchOrLinearSize) : 0;

            printf("Additional flags::\n");
            hdst_.dwFlags & 0x8      ? printf("\tdwFlags::PITCH\n"):0;
            hdst_.dwFlags & 0x20000  ? printf("\tdwFlags::MIPMAP\n") : 0;
            hdst_.dwFlags & 0x80000  ? printf("\tdwFlags::LINEARSIZE\n") : 0;
            hdst_.dwFlags & 0x800000 ? printf("\tdwFlags::DEPTH\n") : 0;

            hdst_.dwCaps & 0x8       ? printf("\tdwCap::COMPLEX : file contains more than one surface\n") : 0;
            hdst_.dwCaps & 0x400000  ? printf("\tdwCap::MIPMAP  : file contains mipmap\n") : 0;

            hdst_.dwCaps2 ? printf("\tdwCaps2 used: %08lx (for cube map/volume flags)\n", hdst_.dwCaps2) : 0;
            printf("--FLAGS--\n");
            hdst_.dwCaps3 ? printf("\tdwCaps3 used: %08lx\n", hdst_.dwCaps3) : 0;
            hdst_.dwCaps4 ? printf("\tdwCaps4 used: %08lx\n", hdst_.dwCaps4) : 0;

            for(int i=0;i<11;i++)
                if (hdst_.dwReserved1[i])
                {
                    dwtostr(hdst_.dwReserved1[i], buf);
                    printf("dwReserved1[%2d] used: %08lx = %s\n", i, hdst_.dwReserved1[i],buf);
                }

            if (hdst_.dwReserved2)
            {
                dwtostr(hdst_.dwReserved2, buf);
                printf("dwReserved2     used: %08lx = %s\n", hdst_.dwReserved2,buf);
            }

            printf("\n\nPIXEL infomations::\nFLAGS::\n");
            hdst_.ddspf.dwFlags & 0x1     ? printf("\tDDPF_ALPHAPIXELS: contains alpha data\n") : 0;
            hdst_.ddspf.dwFlags & 0x2     ? printf("\tDDPF_ALPHA:       contains uncompressed alpha data (legacy)\n") : 0;
            hdst_.ddspf.dwFlags & 0x4     ? printf("\tDDPF_FOURCC:      contains compressed RGB data\n") : 0;
            hdst_.ddspf.dwFlags & 0x40    ? printf("\tDDPF_RGB:         contains uncompressed RGB data\n") : 0;
            hdst_.ddspf.dwFlags & 0x200   ? printf("\tDDPF_YUV:         contains uncompressed YUV data (legacy)\n") : 0;
            hdst_.ddspf.dwFlags & 0x20000 ? printf("\tDDPF_LUMINANCE:   contains luminance data (legacy)\n") : 0;
            printf("--FLAGS--\n");
            dwtostr(hdst_.ddspf.dwFourCC, buf);
            printf("FourCC : %s\n", buf);
            if (hdst_.ddspf.dwFlags & 0x40)
            {
                printf("Number of bits in an RGB : %u bits\n", hdst_.ddspf.dwRGBBitCount);
                printf("Color mask for R : %08lx\n", hdst_.ddspf.dwRBitMask);
                printf("Color mask for G : %08lx\n", hdst_.ddspf.dwGBitMask);
                printf("Color mask for B : %08lx\n", hdst_.ddspf.dwBBitMask);
                printf("Color mask for A : %08lx\n", hdst_.ddspf.dwABitMask);
            }
        }

        return true;
    }
    
public:
    
    ddsProject(const wchar_t *p)
    {
        filepath_ = p;
    }
    ddsProject()
    {
        status_ = none;
        width_ = height_ = 0;
    }
    bool readInfo()
    {//TODO:
        //if (!fpHeader_) { printf("Internal Error: read header first\n"); return false; }
        return this->readHeader();
    }
    inline bool ispow2(size_t x)
    {
        return ((x != 0) && !(x & (x - 1)));
    }

    int getMipmapSize()
    {
        int t = width_ > height_ ? height_ : width_;
        int res = 0;
        while (t & (1 << res++));
        return res;
    }
    bool makeUnsplit()
    {
        FILE* writeFile;
        FILE* dataFile;
        char buf;
        fs::path newFilepath(filepath_), dataFilepath(filepath_);
        DirectX::DDS_HEADER hdst; //= { 124,0x1|0x2|0x4|0x1000, };
        memcpy(&hdst, &hdst_,sizeof(hdst));
        newFilepath.replace_extension(L"unsplit.dds");
        char fragname[3] = ".0";
        fragname[1] += frag_;
        dataFilepath += fragname;

        writeFile = _wfopen(newFilepath.wstring().c_str(), L"wb");
        dataFile = _wfopen(dataFilepath.wstring().c_str(), L"rb");


        hdst.dwMipMapCount = 1; hdst.dwFlags &= ~0x20000;   hdst.dwCaps &= ~0x400008;   //remove unwanted flags for unmipmap
        //hdst.dwHeight = height_;     hdst.dwWidth = width_; hdst.dwMipMapCount = 1;
        //hdst.ddspf.dwFlags = 0x4;   hdst.ddspf.dwFourCC = 0x31545844;

        fwrite(&DirectX::DDS_MAGIC, sizeof(DWORD), 1, writeFile);
        fwrite(&hdst, sizeof(hdst), 1, writeFile);
        //fwrite(&hdst_, sizeof(hdst), 1, writeFile);
        while (fread(&buf, sizeof(buf), 1, dataFile)) fwrite(&buf, sizeof(buf), 1, writeFile);
        fclose(writeFile);
        fclose(dataFile);

        return true;
    }
    bool isUnsplitable()
    {
        fs::path dataFilepath(filepath_);
        if (status_ == mipmapped)
        {
            if (chkFrag(frag_))
                return true;
        }
        else return false;
    }
};
void parseOption(wchar_t* option)
{
    char optinfo[65];
    switch (option[0])
    {
    case 'w':       //overwrite
        sprintf(optinfo, "overwrite");
        wflag = true;
        break;
    case 'v':       //verdose
        sprintf(optinfo, "verdose");
        vflag = true;
        break;
    case 'r':       //recursive
        sprintf(optinfo, "recursive");
        rflag = true;
        break;
    case 'c':       //convert
        sprintf(optinfo, "conversion");
        cflag = true;
        break;
    case 'i':       //info
        sprintf(optinfo, "infomation");
        iflag = true;
        break;
    default:
        sprintf(optinfo, "invalid");
        break;
    }
    printf("%s option enabled\n",optinfo);
}
void parseFile(const wchar_t* path)
{//TODO: check it is really dds file, and find splitted files:
    // entry.path().stem().extension().compare(".dds") == 0
    std::wcout << "file: " << path << '\n';
    ddsProject newfile(path);
    newfile.readInfo();
    if (newfile.isUnsplitable()) newfile.makeUnsplit();
}
void parseDir(const wchar_t* path)
{
    fs::path dirpath = path;
#ifdef _DEBUG
    printf("Dir)");
    std::cout << dirpath.filename() << "::\n";
#endif
    try
    {
        for (fs::directory_entry const& entry : fs::directory_iterator(dirpath)) {
            if (entry.is_regular_file())
            {
                //std::cout <<"Nf" << entry.path().filename() << entry.path().extension()<<"//"<< entry.path().stem().extension() << '\n';
                if (entry.path().extension().compare(".dds") == 0) parseFile(entry.path().wstring().c_str());
            }
            else if (rflag && entry.is_directory()) parseDir(entry.path().wstring().c_str());
        }
    }
    catch (fs::filesystem_error const& e) {
        std::cout << e.what() << '\n';
    }
}

void parseArguments(int argc, wchar_t* argv[])
{//FIXME:
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-') parseOption(&argv[i][1]);
        else if (fs::is_regular_file(fs::status(argv[i])) && fs::path(argv[i]).extension().compare(".dds") == 0) parseFile(&argv[i][0]);
        else if (fs::is_directory(fs::status(argv[i]))) parseDir(&argv[i][0]);
#ifdef _DEBUG
        else printf("None\n");
#endif
    }
}
int wmain(int argc, wchar_t *argv[])
{
    setlocale(LC_ALL, "");  //set locale to system's one to support cjk characters.
#ifdef _DEBUG
    rflag = true;   iflag = true;
#endif
    parseArguments(argc, argv);
    system("pause");
    return 0;
}
