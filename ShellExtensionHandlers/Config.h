
/* Custom File Format Definitions */

/* IMPORTANT: Don't use the same definitions for Preview and Thumbnail handlers */

#if !defined(DEFINE_ICONHANDLER) && !defined(DEFINE_THUMBNAILHANDLER)

/* Define to implement a preview handler */
//#define DEFINE_ICONHANDLER

/* Define to implement a thumbnail handler */
#define DEFINE_THUMBNAILHANDLER

#endif


#if defined(DEFINE_ICONHANDLER)

// {F506475A-9ED1-4FED-A982-FB0FAD54B5F4}
const CLSID CLSID_ShelExtensionHandler =
{ 0xf506475a, 0x9ed1, 0x4fed, { 0xa9, 0x82, 0xfb, 0xf, 0xad, 0x54, 0xb5, 0xf4 } };

// {52633D5E-F00B-497D-98C3-841A82463756}
const GUID APPID_ShelExtensionHandler =
{ 0x52633d5e, 0xf00b, 0x497d, { 0x98, 0xc3, 0x84, 0x1a, 0x82, 0x46, 0x37, 0x56 } };

#define FORMAT_EXTENSION L".wwd"
#define HANDLER_DESCRIPTION L"WapMapIconWWD"
#define APPICATION_DESCRIPTION L"WapMapIconHandlerWWD"

#elif defined(DEFINE_THUMBNAILHANDLER)

// {250D55F5-7C11-41F9-B50F-C28F26530B8E}
const GUID CLSID_ShelExtensionHandler =
{ 0x250d55f5, 0x7c11, 0x41f9, { 0xb5, 0xf, 0xc2, 0x8f, 0x26, 0x53, 0xb, 0x8e } };

// {6D39B2CD-22F4-4D85-AE1F-11EE42F607B8}
const GUID APPID_ShelExtensionHandler =
{ 0x6d39b2cd, 0x22f4, 0x4d85, { 0xae, 0x1f, 0x11, 0xee, 0x42, 0xf6, 0x7, 0xb8 } };

#define FORMAT_EXTENSION L".wwd"
#define HANDLER_DESCRIPTION L"WapMapThumbnailWWD"
#define APPICATION_DESCRIPTION L"WapMapThumbnailHandlerWWD"

#endif
