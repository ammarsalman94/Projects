// dllmain.h : Declaration of module class.

class CTextSearchComModule : public ATL::CAtlDllModuleT< CTextSearchComModule >
{
public :
	DECLARE_LIBID(LIBID_TextSearchComLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TEXTSEARCHCOM, "{31F65FDD-5669-4E3A-A6C1-2698B1229C06}")
};

extern class CTextSearchComModule _AtlModule;
