#include<bits/stdc++.h>
#include <unistd.h>    // For fork() and execv()
#include <sys/wait.h>
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

bool findExecute(string cmnd, vector<string> &args, bool istypecmnd){
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
                          if(!istypecmnd){
                              // Build the arguments array for execv
                              // execv expects: [0] = program path/name, [1..n] = args, [n+1] = NULL
                              vector<char*> c_args;
                              // c_args.push_back(const_cast<char*>(cmnd)); // The first arg is conventionally the command itself

                              for (const auto& arg : args) {
                                  c_args.push_back(const_cast<char*>(arg.c_str()));
                              }
                              c_args.push_back(nullptr); // Array must be null-terminated
                            
                              pid_t pid = fork();
                            
                              if (pid == 0) {
                                  // Child process: Execute the command
                                  if (execv(entry.path().c_str(), c_args.data()) == -1) {
                                      perror("Execution failed");
                                      std::exit(EXIT_FAILURE); 
                                  }
                              } else if (pid > 0) {
                                  // Parent process: Wait for the child to finish
                                  int status;
                                  waitpid(pid, &status, 0);
                              } else {
                                  // Fork failed
                                  perror("Fork failed");
                                  return false;
                              }
                          }
                           executablefileavailable=true;
                           exit=true;
                           path=entry.path();
                        }
                    }
                    break;
                  }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "  Error reading directory: " << e.what() << "\n";
            }
            if(exit)break;
        }
        if(istypecmnd){
          if(executablefileavailable){
            cout<<cmnd<<" is "<<path<<endl;
          }else{
            cout<<cmnd<<": not found"<<endl;
          }
        }
        return executablefileavailable;
}

int main() {
  for(;;){
  cout << "$ ";
  string s;
  getline(cin, s);
  if(s=="exit")break;
  else if(s=="pwd"){
    cout<<fs::current_path()<<endl;
  }
  else if(s.substr(0,5)=="echo "){
    cout<<s.substr(5)<<endl;
  }else if(s.substr(0,5)=="type "){
    string cmnd=s.substr(5);
    if(cmnd=="echo" || cmnd=="type" || cmnd=="exit"){
        cout<<cmnd<<" is a shell builtin"<<endl;
    }else{
        vector<string> v;
        findExecute(cmnd,v,true);
    }
  }else{

    stringstream ss(s);
    string word;
    vector<string> words;
    // Extract words using the >> operator, which naturally splits at spaces
    while (ss >> word) {
        words.push_back(word);
    }
    bool foundexecutable=findExecute(words[0],words,false);
    if(!foundexecutable)cout<<s<<": command not found"<<endl;
  }
  }
}
