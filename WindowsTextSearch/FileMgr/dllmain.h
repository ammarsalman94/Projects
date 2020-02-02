// dllmain.h : Declaration of module class.

class CFileMgrModule : public ATL::CAtlDllModuleT< CFileMgrModule >
{
public :
	DECLARE_LIBID(LIBID_FileMgrLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FILEMGR, "{601EAB7D-CC4D-4158-93AA-768AFECE151C}")
};

extern class CFileMgrModule _AtlModule;
