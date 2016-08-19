
/* Custom File Format Definitions */

/* Build using Microsoft GUIDGEN Application (option 3) */
// {1BD58F32-B48D-4C1D-A442-28A70B49A59B}
const CLSID CLSID_ShelExtensionHandler = 
{ 0x1bd58f32, 0xb48d, 0x4c1d, { 0xa4, 0x42, 0x28, 0xa7, 0xb, 0x49, 0xa5, 0x9b } };


/* Build using Microsoft GUIDGEN Application (option 3) */
// {08B403B0-BCE3-4E8E-9E60-C929EB69A45F}
const GUID APPID_ShelExtensionHandler = 
{ 0x8b403b0, 0xbce3, 0x4e8e, { 0x9e, 0x60, 0xc9, 0x29, 0xeb, 0x69, 0xa4, 0x5f } };


//#define FORMAT_EXTENSION L".cgm"
//#define FORMAT_DESCRIPTION L"CgmShelExtPreviewHandler"
//#define FORMAT_EXTENSION L".emf"
//#define FORMAT_DESCRIPTION L"EmfShelExtPreviewHandler"
#define FORMAT_EXTENSION L".tga"
#define FORMAT_DESCRIPTION L"TgaShelExtPreviewHandler"

#define APPICATION_DESCRIPTION L"IupCdShellExtPreviewHandler"

/* Define to implement a preview handler */
#define DEFINE_PREVIEWHANDLER

/* Define to implement a thumbnail handler */
//#define DEFINE_THUMBNAILHANDLER
