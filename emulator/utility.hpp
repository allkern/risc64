#pragma once

#include <iomanip>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <commdlg.h>
#endif

namespace machine {
	namespace utility {
        // Round a float to 1 decimal place
        float round(float var) { 
            float value = (int)(var * 10 + .5); 
            return (float)value / 10; 
        }

        // Get a zero-filled hexadecimal representation of value
		template <class T> static const std::string hex(T value) {
            std::ostringstream ss;
            ss << std::setw(sizeof(T)*2) << std::setfill('0') << std::hex << value;
            return ss.str();
        }
#ifdef _WIN32
        // Open a file save dialog
        inline const std::string open_file_save_dialog(const std::string& str_title, const std::string& str_filter, const std::string& str_def_ext) {
            char file[500];
            const char *filter = str_filter.c_str(),
                       *title = str_title.c_str(),
                       *def_ext = str_def_ext.c_str();
            file[0] = '\x0';

            OPENFILENAMEA ofn;
            ofn.lStructSize          = sizeof(OPENFILENAMEA);
            ofn.hwndOwner            = NULL;
            ofn.hInstance            = NULL;
            ofn.lpstrFilter          = filter;
            ofn.lpstrCustomFilter    = NULL;
            ofn.nMaxCustFilter       = NULL;
            ofn.nFilterIndex         = NULL;
            ofn.lpstrFile            = file;
            ofn.nMaxFile             = 500;
            ofn.lpstrFileTitle       = NULL;
            ofn.nMaxFileTitle        = NULL;
            ofn.lpstrInitialDir      = NULL;
            ofn.lpstrTitle           = title;
            ofn.Flags                = NULL;
            ofn.nFileOffset          = NULL;
            ofn.nFileExtension       = NULL;
            ofn.lpstrDefExt          = def_ext;
            ofn.lCustData            = NULL;
            ofn.lpfnHook             = NULL;
            ofn.lpTemplateName       = NULL;
            GetSaveFileName(&ofn);

            return std::string(file);
        }
#endif
	}
}