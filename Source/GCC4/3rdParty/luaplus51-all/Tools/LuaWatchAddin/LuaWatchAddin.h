
#define ADDIN_API __declspec(dllexport)

extern "C" ADDIN_API HRESULT WINAPI AddIn_LuaStackIndex( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved );
extern "C" ADDIN_API HRESULT WINAPI AddIn_LuaObject( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved );
extern "C" ADDIN_API HRESULT WINAPI AddIn_LuaTableIterator( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved );
extern "C" ADDIN_API HRESULT WINAPI AddIn_TString( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved );


