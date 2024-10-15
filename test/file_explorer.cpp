#include <shlobj.h>

int main()
{
  TCHAR path[MAX_PATH];
    
        const char* path_param = saved_path.c_str();

  BROWSEINFO bi = { 0 };
  bi.lpszTitle = ("Browse for folder...");
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  bi.lpfn = BrowseCallbackProc;
  //bi.lParam = (LPARAM)path_param;

    
  LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
}