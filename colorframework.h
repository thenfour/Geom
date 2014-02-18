/*
  This is the framework used for dealing with color.  Specific colorspace 

  "colorspace" - aka color type, aka color provider.  Some colorspaces arent necessarily REAL colorspaces,
    like CS_Invalid, or CS_IndustryPalette.  All colorspaces either use a list of Colorants, or some
    default data array (byte, word, and dword are provided).  All this fixed-type, fixed-length stuff is
    a sacrifice to allow for faster routines.  I assume that any self-respecting color can use these 32 bytes
    to describe itself.

  "colorant" - aka color component.  Each color is made of several colorants.  For an RGB color, the 3 colorants
    are Red, Green, and Blue.  ALL "normal" colors are made of colorants.  Only the special colors used
    in this library dont use colorants - such as CS_IndustryPalette or CS_Invalid.
*/


#pragma once


#include <string>
#include <vector>


namespace Colors
{
  typedef float Colorant;

  typedef unsigned __int8 ColorSpaceID;

  const ColorSpaceID CS_Invalid = 0;

  typedef long ConversionResult;
  static const long CR_InGamut = 0;
  static const long CR_OutOfGamut = 1;
  static const long CR_ConversionFailed = 1;
  static const long CR_Error = CR_ConversionFailed;

  // smarter replacement for RGBQUAD.  note this stuff only works on little-endian.
  typedef DWORD RgbPixel;

  inline BYTE R(RgbPixel d)
  {
    return static_cast<BYTE>((d & 0x00FF0000) >> 16);
  }

  inline BYTE G(RgbPixel d)
  {
    return static_cast<BYTE>((d & 0x0000FF00) >> 8);
  }

  inline BYTE B(RgbPixel d)
  {
    return static_cast<BYTE>((d & 0x000000FF));
  }

  inline RgbPixel MakeRgbPixelB(BYTE r, BYTE g, BYTE b)
  {
    return static_cast<RgbPixel>((r << 16) | (g << 8) | (b));
  }

  template<typename T>
  inline RgbPixel MakeRgbPixel(const T& r, const T& g, const T& b)
  {
    return MakeRgbPixelB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));
  }

  inline COLORREF RgbPixelToCOLORREF(RgbPixel x)
  {
    return RGB(R(x), G(x), B(x));
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Raw single color data storage.  Each colorspace will use these however it wants.  All color
  // spaces do have some requirements in order to standardize how this works:
  // 1) all colorants are from 0-1 range.  they can be outside of these bounds (meaning out of gamut),
  //    but no boundschecking is performed so its not a good idea.
  // 2) all colorants are indexed in order that they are presented in ColorSpaceInfo.  This way
  //    external sources can access meaningful colorant info directly, without having to marshal through
  //    colorspace-specific routines.
  class ColorData
  {
  public:
    ColorData() {}
    ColorData(const ColorData& r)
    {
      for(long i = 0; i < MaxColorants; i ++)
      {
        m_Colorants[i] = r.m_Colorants[i];
      }
    }
    ColorData& operator = (const ColorData& r)
    {
      for(long i = 0; i < MaxColorants; i ++)
      {
        m_Colorants[i] = r.m_Colorants[i];
      }
      return *this;
    }

    // maximum number of colorants that can be used for a single color, system-wide
    static const long MaxColorants = 8;
    static const long MaxByteData = 8;

    union
    {
      Colorant m_Colorants[MaxColorants];
      unsigned __int8 m_ByteData[MaxByteData];
      unsigned __int16 m_WordData[MaxByteData/2];
      unsigned __int32 m_DwordData[MaxByteData/4];
    };
  };

  //////////////////////////////////////////////////////////////////////////////////////////
  // when registering new colorspaces, this struct is used to provide information 
  // about a colorant.
  class ColorantInfo
  {
  public:
    ColorantInfo() { }

    ColorantInfo(const std::string& _Abbreviation, const std::string& _LongName, const std::string& _Desc)
    {
      Abbreviation = _Abbreviation;
      LongName = _LongName;
      Description = _Desc;
    }

    ColorantInfo(const ColorantInfo& r) :
      Abbreviation(r.Abbreviation),
      LongName(r.LongName),
      Description(r.Description)
    { }

    ColorantInfo& operator = (const ColorantInfo& r)
    {
      Abbreviation = r.Abbreviation;
      LongName = r.LongName;
      Description = r.Description;
      return *this;
    }

    std::string Abbreviation;
    std::string LongName;
    std::string Description;
  };

  //////////////////////////////////////////////////////////////////////////////////////////
  // fill one of these out and pass it to ColorManager::RegisterColorSpace()
  class ColorSpaceInfo
  {
  public:
    typedef RgbPixel (__stdcall* ToRGBFastProc)(const ColorData&);// proc for QUICKLY converting to pixel format
    typedef ConversionResult (__stdcall* ConvertToProc)(ColorSpaceID, ColorData&);// less speed intensive conversion function
    typedef void (__stdcall* InitNewProc)(ColorData&);// initializes a new color
    typedef std::vector<ColorantInfo> ColorantList;

    ColorSpaceInfo() { }

    ColorSpaceInfo(const ColorSpaceInfo& r) :
      id(r.id),
      nColorants(r.nColorants),
      bUsesColorants(r.bUsesColorants),
      Name(r.Name),
      Description(r.Description),
      Colorants(r.Colorants),
      pToRGBFast(r.pToRGBFast),
      pConvertTo(r.pConvertTo),
      pInitNew(r.pInitNew)
    {
    }

    ColorSpaceInfo& operator = (const ColorSpaceInfo& r)
    {
      id = r.id;
      nColorants = r.nColorants;
      bUsesColorants = r.bUsesColorants;
      Name = r.Name;
      Description = r.Description;
      Colorants = r.Colorants;
      pToRGBFast = r.pToRGBFast;
      pConvertTo = r.pConvertTo;
      pInitNew = r.pInitNew;
      return *this;
    }

    ColorSpaceID id;
    long nColorants;
    bool bUsesColorants;
    std::string Name;
    std::string Description;
    std::vector<ColorantInfo> Colorants;
    ToRGBFastProc pToRGBFast;
    ConvertToProc pConvertTo;
    InitNewProc pInitNew;
  };

  //////////////////////////////////////////////////////////////////////////////////////////
  // All support stuff for CS_Invalid
  inline void __stdcall InvalidInitNew(ColorData& c)
  {
    c.m_DwordData[0] = 0;
  }

  inline ConversionResult __stdcall InvalidConvertTo(ColorSpaceID destid, ColorData& dat)
  {
    return CR_ConversionFailed;
  }

  inline RgbPixel __stdcall InvalidToRGBFast(const ColorData& dat)
  {
    return MakeRgbPixel(0,0,0);
  }

  inline ColorSpaceInfo InvalidGetInfo()
  {
    ColorSpaceInfo r;
    r.id = CS_Invalid;
    r.nColorants = 0;
    r.bUsesColorants = false;
    r.Name = "Invalid";
    r.Description = "Invalid";
    // no colorants to set
    r.pToRGBFast = InvalidToRGBFast;
    r.pConvertTo = InvalidConvertTo;
    r.pInitNew = InvalidInitNew;
    return r;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // This holds data about the different registered colorspaces.  Its used to basically store
  // static data to do conversions and things.  This object will hold all the info needed for
  // colorspaces, then individual colors can call on it to do things like convert to rgb, etc.
  class ColorManager
  {
  public:
    ColorManager()
    {
      // initialize with some default crap about CS_Invalid
      RegisterColorSpace(InvalidGetInfo());
    }

    ColorManager(const ColorManager& r) :
      ColorSpaces(r.ColorSpaces)
    {
    }

    ColorManager& operator = (const ColorManager& r)
    {
      ColorSpaces = r.ColorSpaces;
      return *this;
    }

    bool RegisterColorSpace(const ColorSpaceInfo& csi)
    {
      // TODO: check to see if the colorspace is already registered, and other validation
      ColorSpaces.push_back(csi);
      return true;
    }

    ColorSpaceInfo* FindColorSpaceInfo(ColorSpaceID id)
    {
      ColorSpaceInfo* r = 0;
      // TODO: speed this up
      ColorSpaceList::iterator it;
      for(it = ColorSpaces.begin(); it != ColorSpaces.end(); ++it)
      {
        if(it->id == id)
        {
          r = &(*it);
          break;
        }
      }
      return r;
    }

  private:
    typedef std::vector<ColorSpaceInfo> ColorSpaceList;
    ColorSpaceList ColorSpaces;
  };

  //////////////////////////////////////////////////////////////////////////////////////////
  // variant color, basically.  This represents 1 item in a color palette.
  class ColorSpec
  {
  public:
    ColorSpec() :
      m_pManager(0),
      m_pcsi(0),
      m_csid(CS_Invalid)
    {
    }

    ColorSpec(ColorManager* mgr) :
      m_pManager(mgr),
      m_pcsi(0),
      m_csid(CS_Invalid)
    {
    }

    ColorSpec(const ColorSpec& r) :
      m_pManager(r.m_pManager),
      m_pcsi(r.m_pcsi),
      m_csid(r.m_csid),
      m_data(r.m_data)
    {
    }

    inline ColorSpec& operator = (const ColorSpec& r)
    {
      m_pManager = r.m_pManager;
      m_pcsi = r.m_pcsi;
      m_csid = r.m_csid;
      m_data = r.m_data;
      return *this;
    }

    inline void SetManager(ColorManager* p)
    {
      m_pManager = p;
    }

    inline const ColorSpaceID& GetColorSpaceID() const
    {
      return m_csid;
    }

    inline bool InitNew(ColorSpaceID n)
    {
      bool r = false;
      ColorSpaceInfo* pNewCSI = m_pManager->FindColorSpaceInfo(n);
      if(pNewCSI)
      {
        m_pcsi = pNewCSI;
        m_pcsi->pInitNew(m_data);
        m_csid = pNewCSI->id;
        r = true;
      }
      return r;
    }

    inline ConversionResult ConvertToColorSpace(ColorSpaceID dest)
    {
      ConversionResult r = CR_ConversionFailed;
      ColorSpaceInfo* pNewCSI = m_pManager->FindColorSpaceInfo(dest);
      if(pNewCSI)
      {
        r = m_pcsi->pConvertTo(dest, m_data);
        m_pcsi = pNewCSI;
      }
      return r;
    }

    inline long GetColorantCount() const
    {
      return m_pcsi->nColorants;
    }

    inline bool UsesColorants() const
    {
      return m_pcsi->bUsesColorants;
    }

    inline const std::string& GetColorantAbbreviation(long n) const
    {
      return m_pcsi->Colorants[n].Abbreviation;
    }

    inline const std::string& GetColorantLongName(long n) const
    {
      return m_pcsi->Colorants[n].LongName;
    }

    inline const std::string& GetColorantDescription(long n) const
    {
      return m_pcsi->Colorants[n].Description;
    }

    inline const Colorant& GetColorant(long n) const
    {
      return m_data.m_Colorants[n];
    }

    inline Colorant& GetColorant(long n)
    {
      return m_data.m_Colorants[n];
    }

    inline const std::string& GetColorSpaceName() const
    {
      return m_pcsi->Name;
    }

    inline const std::string& GetColorSpaceDescription() const
    {
      return m_pcsi->Description;
    }

    inline RgbPixel GetRGBFast() const
    {
      return m_pcsi->pToRGBFast(m_data);
    }

  private:
    ColorManager* m_pManager;
    ColorSpaceInfo* m_pcsi;// every time it changes type, this is updated to reference an entry in m_pManager.
    ColorSpaceID m_csid;
    ColorData m_data;
  };
}

