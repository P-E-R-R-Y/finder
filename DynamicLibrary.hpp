#ifndef DYNAMIC_LIBRARY_HPP
#define DYNAMIC_LIBRARY_HPP

#include <string>

#ifdef WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


class DynamicLibrary {
    public:
        DynamicLibrary(std::string path) : _path(path) {
            #if defined(WIN32)
                _lib = LoadLibraryA(path.c_str());
            #else
                _lib = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            #endif
            
            if (!_lib) {
                this->HandleError();
            }
        }

        ~DynamicLibrary() {
            int ret = 0;
            #if defined(WIN32)
                if (!FreeLibrary(_lib)) {
                    this->handleError();
                }
            #else
                if (dlclose(_lib) != 0) {
                    this->HandleError();
                }
            #endif
        }

        template <typename T>
        T GetSymbol(const std::string& name) {
            static_assert(std::is_pointer<T>::value, "T must be a pointer");

            T symbol = nullptr;

            #if defined(WIN32)
                symbol = reinterpret_cast<T>(GetProcAddress(_lib, name.c_str())); 
            #else
                symbol =  reinterpret_cast<T>(dlsym(_lib, name.c_str()));
            #endif
            if (!symbol) {
                this->handleError();
            }
            return symbol;
        }

        bool IsReady() const {
            return _lib != nullptr;
        }
        
        std::string GetError() const {
            #if defined(WIN32)
                return _lastError;
            #else
                return _lastError;
            #endif
        }

    private:
        void HandleError() {
            #if defined(WIN32)
                DWORD error = GetLastError();
                if (error == 0) {
                    _lastError.clear();
                    return;
                }
                SetLastError(0);
                LPWSTR lpMsgBuf = NULL;
                
                DWORD size = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPWSTR>(&lpMsgBuf),
                    0, 
                    nullptr);
                if (lpMsgBuf) {
                    _lastError = std::string(lpMsgBuf, size);
                    LocalFree(lpMsgBuf);
                    return;
                }
                _lastError = "Unknown error";
            #else
                _lastError = std::string(dlerror());
            #endif
        }

    protected:
        std::string _path;
        std::string _lastError;
    
    #if defined(WIN32)
        HMODULE _lib;
    #else
        void *_lib;
    #endif
};

#endif // DYNAMIC_LIBRARY_HPP