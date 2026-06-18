#include<bits/stdc++.h>
using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
    const char PATH_DELIMITER = ';';
#else
    const char PATH_DELIMITER = ':';
#endif

// Helper function to check if a file has executable permissions
bool isExecutable(const fs::path& filePath) {
    try {
        auto perms = fs::status(filePath).permissions();
        
        // Check if owner, group, or others have execute bit enabled
        return (perms & (fs::perms::owner_exec | 
                         fs::perms::group_exec | 
                         fs::perms::others_exec)) != fs::perms::none;
    } catch (const fs::filesystem_error&) {
        return false; // Fail gracefully if permissions can't be read
    }
}

int main() {
  for(;;){
  cout << "$ ";
  string s;
  getline(cin, s);
  if(s=="exit")break;
  else if(s.substr(0,5)=="echo "){
    cout<<s.substr(5)<<endl;
  }else if(s.substr(0,5)=="type "){
    string cmnd=s.substr(5);
    if(cmnd=="echo" || cmnd=="type" || cmnd=="exit"){
        cout<<cmnd<<" is a shell builtin"<<endl;
    }else{
        const char* pathEnv = std::getenv("PATH");
        std::string pathStr(pathEnv);
        std::stringstream ss(pathStr);
        std::string singlePath;
        bool executablefileavailable=false;
        bool exit=false;
        string path;
        while (std::getline(ss, singlePath, PATH_DELIMITER)) {
            if (singlePath.empty()) continue;
            fs::path dirPath(singlePath);

            // 3. Skip if the path is invalid or is not a directory
            if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
                continue; 
            }
          
            // 4. Iterate over files within the target directory
            try {
                for (const auto& entry : fs::directory_iterator(dirPath)) {
                  if(entry.path().filename().string()==cmnd){
                    if (fs::is_regular_file(entry.status())) {
                        bool executable = isExecutable(entry.path());

                        if(executable){
                           executablefileavailable=true;
                           path=entry.path();
                        }
                    }
                    exit=true;
                    break;
                  }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "  Error reading directory: " << e.what() << "\n";
            }
            if(exit)break;
        }
        if(executablefileavailable){
          cout<<cmnd<<" is "<<path<<endl;
        }else{
          cout<<cmnd<<": not found"<<endl;
        }
    }
  }else{
    cout<<s<<": command not found"<<endl;
  }
  }
}
