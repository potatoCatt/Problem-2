
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <iostream>
#include <windows.h>
#include <string.h>
#include <thread>
#include <utility>
#include <queue>
#include <Shobjidl.h> 
#include <shellapi.h> 
#include <experimental/filesystem>
#include <fstream>





BOOL deleteFileOrFolder(LPCWSTR fileOrFolderPath) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
       
        MessageBox(NULL, L"Couldn't initialize COM library", L"Whoops", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return FALSE;
    }
    
    IFileOperation* fileOperation;
    hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&fileOperation));
    if (FAILED(hr)) {
        
        MessageBox(NULL, L"Couldn't CoCreateInstance", L"Whoops", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return FALSE;
    }
    hr = fileOperation->SetOperationFlags(FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI);
    if (FAILED(hr)) {
       
        MessageBox(NULL, L"Couldn't add flags", L"Whoops", MB_OK | MB_ICONERROR);
        fileOperation->Release();
        CoUninitialize();
        return FALSE;
    }
    IShellItem* fileOrFolderItem = NULL;
    hr = SHCreateItemFromParsingName(fileOrFolderPath, NULL, IID_PPV_ARGS(&fileOrFolderItem));
    if (FAILED(hr)) {
       
        MessageBox(NULL, L"Couldn't get file into an item", L"Whoops", MB_OK | MB_ICONERROR);
        fileOrFolderItem->Release();
        fileOperation->Release();
        CoUninitialize();
        return FALSE;
    }
    hr = fileOperation->DeleteItem(fileOrFolderItem, NULL); 
    fileOrFolderItem->Release();
    if (FAILED(hr)) {
        
        MessageBox(NULL, L"Failed to mark file/folder item for deletion", L"Whoops", MB_OK | MB_ICONERROR);
        fileOperation->Release();
        CoUninitialize();
        return FALSE;
    }
    hr = fileOperation->PerformOperations();
    fileOperation->Release();
    CoUninitialize();
    if (FAILED(hr)) {
        
        MessageBox(NULL, L"failed to carry out delete", L"Whoops", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    return TRUE;
}


HRESULT CopyItem(__in PCWSTR pszSrcItem, __in PCWSTR pszDest, PCWSTR pszNewName)
{
  
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOperation* pfo;

        hr = CoCreateInstance(CLSID_FileOperation,
            NULL,
            CLSCTX_ALL,
            IID_PPV_ARGS(&pfo));
        if (SUCCEEDED(hr))
        {
         
            hr = pfo->SetOperationFlags(FOF_NO_UI);
            if (SUCCEEDED(hr))
            {
               
                IShellItem* psiFrom = NULL;
                hr = SHCreateItemFromParsingName(pszSrcItem,
                    NULL,
                    IID_PPV_ARGS(&psiFrom));
                if (SUCCEEDED(hr))
                {
                    IShellItem* psiTo = NULL;

                    if (NULL != pszDest)
                    {
                        hr = SHCreateItemFromParsingName(pszDest,
                            NULL,
                            IID_PPV_ARGS(&psiTo));
                    }

                    if (SUCCEEDED(hr))
                    {
                        
                        hr = pfo->CopyItem(psiFrom, psiTo, pszNewName, NULL);

                        if (NULL != psiTo)
                        {
                            psiTo->Release();
                        }
                    }

                    psiFrom->Release();
                }

                if (SUCCEEDED(hr))
                {
                  
                    hr = pfo->PerformOperations();
                }
            }

           
            pfo->Release();
        }

        CoUninitialize();
    }
    return hr;
}





int main(int argc, char* argv[])
{
    
    int interval = atoi(argv[3]);

    setlocale(LC_ALL, "Russian");
    std::string strpathS(argv[1]);
    std::string strpathR(argv[2]);
    std::wstring pathS = std::wstring(strpathS.begin(), strpathS.end());
    std::wstring pathR = std::wstring(strpathR.begin(), strpathR.end());


    std::experimental::filesystem::remove_all(argv[2]);
    std::experimental::filesystem::copy(argv[1], argv[2]);

    const wchar_t* WpathR = pathR.c_str();
    const wchar_t* WpathS = pathS.c_str();
    std::queue<std::pair<std::wstring, std::wstring>> LogOfChanges;
   
   


   
    std::thread t1([&]() {

        HANDLE SourceDir = CreateFile(WpathS,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);
        HANDLE ReplicaDir = CreateFile(WpathR,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);
        
        OVERLAPPED overlapped;
        overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

        uint8_t change_buf[1024];
        BOOL success = ReadDirectoryChangesW(
            SourceDir, change_buf, 1024, TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL, &overlapped, NULL);

        while (true) {
            DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

            if (result == WAIT_OBJECT_0) {
                DWORD bytes_transferred;
                GetOverlappedResult(SourceDir, &overlapped, &bytes_transferred, FALSE);

                FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;
                std::wstring OldName;

                for (;;) {
                    DWORD name_len = event->FileNameLength / sizeof(wchar_t);

                    switch (event->Action) {
                    case FILE_ACTION_ADDED: {
                       
                      
                        int counter = 0;
                        std::wstring pathF;
                        while (counter < name_len)
                        {
                            pathF.push_back(event->FileName[counter]);
                            counter++;
                        }
                       
                          
                            std::wstring action(L"add");
                            std::pair<std::wstring, std::wstring> FileAction;
                            FileAction.first = pathF;
                            FileAction.second = action;
                            LogOfChanges.push(FileAction);
                      
                    } break;

                    case FILE_ACTION_REMOVED: {
                        
                      
                        int counter = 0;
                        std::wstring pathF;
                        while (counter < name_len)
                        {
                            pathF.push_back(event->FileName[counter]);
                            counter++;
                        }
                     
                            
                            std::wstring action(L"del");
                            std::pair<std::wstring, std::wstring> FileAction;
                            FileAction.first = pathF;
                            FileAction.second = action;
                            LogOfChanges.push(FileAction);
                      

                    } break;

                    case FILE_ACTION_MODIFIED: {
                        
                      
                        int counter = 0;
                        std::wstring pathF;
                        while (counter < name_len)
                        {
                            pathF.push_back(event->FileName[counter]);
                            counter++;
                        }
                    
                          
                            std::wstring action(L"mod");
                            std::pair<std::wstring, std::wstring> FileAction;
                            FileAction.first = pathF;
                            FileAction.second = action;
                            LogOfChanges.push(FileAction);
                     
                    } break;

                    case FILE_ACTION_RENAMED_OLD_NAME: {
                        
                        int counter = 0;
                        while (counter < name_len)
                        {
                            OldName.push_back(event->FileName[counter]);
                            counter++;
                        }
                        
                    } break;

                    case FILE_ACTION_RENAMED_NEW_NAME: {
                        
                        std::wstring NewName;
                        int counter = 0;
                        while (counter < name_len)
                        {
                            NewName.push_back(event->FileName[counter]);
                            counter++;
                        }
                     
                            std::wstring action(L"oldName");
                            std::pair<std::wstring, std::wstring> FileAction;
                            FileAction.first = OldName;
                            FileAction.second = action;
                            LogOfChanges.push(FileAction);
                            
                            std::wstring action2(L"newName");
                            std::pair<std::wstring, std::wstring> FileAction2;
                            FileAction2.first = NewName;
                            FileAction2.second = action;
                            LogOfChanges.push(FileAction2);
                           
                            
                   
                    } break;

                    default: {
                        printf("Unknown action!\n");
                    } break;
                    }

                   
                    if (event->NextEntryOffset) {
                        *((uint8_t**)&event) += event->NextEntryOffset;
                    }
                    else {
                        break;
                    }
                }

                BOOL success = ReadDirectoryChangesW(
                    SourceDir, change_buf, 1024, TRUE,
                    FILE_NOTIFY_CHANGE_FILE_NAME |
                    FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                    NULL, &overlapped, NULL);

            }

        }


        });
        
        std::thread t2([&]()
            {
                while (true)
                {
                   
                    while (LogOfChanges.size()!=0)
                    {
                        std::pair<std::wstring, std::wstring> ActionFromLog = LogOfChanges.front();
                        LogOfChanges.pop();
                        if (ActionFromLog.second == L"add")
                        {
                            std::wstring tempPathS = pathS;
                            std::wstring tempPathR = pathR;
                            std::wstring name= ActionFromLog.first;
                            tempPathS += L"\\";
                          
                            tempPathS += ActionFromLog.first;
                            std::ofstream log_out;
                            log_out.open(argv[4], std::ios::app);
                            log_out << "Added " << std::string(name.begin(), name.end())
                                   << " to " << std::string(tempPathR.begin(), tempPathR.end())<<"\n";
                            log_out.close();
                            std::wcout << "Added " << name << " to " << tempPathR << std::endl;
                    

                       const wchar_t* PathS = tempPathS.c_str();
                        const wchar_t* PathR = tempPathR.c_str();
                        const wchar_t* Name = name.c_str();

                        CopyItem(PathS, PathR,Name);
                        }
                        if (ActionFromLog.second == L"del")
                        {
                            std::wstring tempPathR = pathR;
                            tempPathR += L"\\";
                            tempPathR += ActionFromLog.first;
                          
                            std::ofstream log_out;
                            log_out.open(argv[4], std::ios::app);
                            log_out << "Removed " << std::string(tempPathR.begin(), tempPathR.end())<<"\n";
                            log_out.close();
                             
                            std::wcout << "Removed " << tempPathR << std::endl;


                            const wchar_t* PathR = tempPathR.c_str();    
                            deleteFileOrFolder(PathR);
                        }
                        if (ActionFromLog.second == L"mod")
                        {
                            std::wstring tempPathR = pathR;
                            tempPathR += L"\\";
                            tempPathR += ActionFromLog.first;


                            std::ofstream log_out;
                            log_out.open(argv[4], std::ios::app);
                            log_out << "Modified " << std::string(tempPathR.begin(), tempPathR.end()) << "\n";
                            log_out.close();

                            std::wcout << "Modified " << tempPathR << std::endl;


                            
                            const wchar_t* PathR = tempPathR.c_str();
                            deleteFileOrFolder(PathR);
                            
                            std::wstring tempPathS = pathS;
                            std::wstring tempPathR1 = pathR;
                            std::wstring name = ActionFromLog.first;
                            tempPathS += L"\\";
                            tempPathS += ActionFromLog.first;
                            
                      



                            const wchar_t* PathS = tempPathS.c_str();
                            const wchar_t* PathR1 = tempPathR1.c_str();
                            const wchar_t* Name = name.c_str();

                            
                            CopyItem(PathS, PathR1, Name);
                           
                        }
                        if (ActionFromLog.second == L"oldName")
                        {
                            std::wstring tempPathR = pathR;
                            tempPathR += L"\\";
                            std::wstring NewPathR = tempPathR;
                            tempPathR += ActionFromLog.first;
                            ActionFromLog = LogOfChanges.front();
                            LogOfChanges.pop();
                            NewPathR += ActionFromLog.first;
                            
                            std::ofstream log_out;
                            log_out.open(argv[4], std::ios::app);
                            log_out << "Renamed from " << std::string(tempPathR.begin(), tempPathR.end()) 
                                    <<" to "<< std::string(NewPathR.begin(), NewPathR.end())<< "\n";
                            log_out.close();

                            std::wcout << "Renamed from " << tempPathR<<" to "<< NewPathR<< std::endl;




                            MoveFile(tempPathR.c_str(), NewPathR.c_str());
                        }
                      
                    }
                  
                    Sleep(1000*interval);
                }
            });
        t1.join();
        t2.join();
}

