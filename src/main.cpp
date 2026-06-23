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

std::vector<std::string> tokenizeString(const std::string& input) {
    std::vector<std::string> tokens;
    std::string currentToken = "";
    bool inQuotes = false;
    bool hasContent = false; // Tracks if the current token has any characters added

    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];

        if (ch == '\'') {
            // Toggle quote state
            inQuotes = !inQuotes;
            // Mark that this token is active (handles cases like empty quotes '' or "" 
            // where you might want to preserve an empty argument)
            hasContent = true; 
        } 
        else if (inQuotes) {
            // Inside quotes: keep all spaces and characters
            currentToken += ch;
            hasContent = true;
        } 
        else {
            // Outside quotes
            if (ch == ' ') {
                // If we hit a space and the current token has content, push it
                if (hasContent) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                    hasContent = false;
                }
                // Multiple consecutive spaces outside quotes are naturally ignored
            } else {
                currentToken += ch;
                hasContent = true;
            }
        }
    }

    // Push the very last token if the string didn't end with a space
    if (hasContent) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

string parseString(const string& input) {
    string result = "";
    bool inQuotes = false;
    bool lastWasSpace = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];

        if (ch == '\'') {
            // Toggle the quote state
            inQuotes = !inQuotes;
            // Reset space tracking when entering/leaving quotes to handle boundaries
            lastWasSpace = false; 
        } 
        else if (inQuotes) {
            // Inside quotes: preserve all characters exactly as they are
            result += ch;
        } 
        else {
            // Outside quotes: handle spaces and standard characters
            if (ch == ' ') {
                if (!lastWasSpace) {
                    result += ' ';
                    lastWasSpace = true;
                }
                // If lastWasSpace is true, consecutive spaces are ignored (collapsed)
            } else {
                result += ch;
                lastWasSpace = false;
            }
        }
    }

    return result;
}

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
                                  c_args.push_back(const_cast<char*>(parseString(arg).c_str()));
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
    cout<<fs::current_path().string()<<endl;
  }else if(s.substr(0,3)=="cd "){
    try {
        if(s.substr(3)=="~"){
          const char* pathEnv = getenv("HOME");
          string pathStr(pathEnv);
          fs::current_path(pathStr);
        }else{
          fs::current_path(parseString(s.substr(3)));
        }
    } 
    catch (const fs::filesystem_error& e) {
        cout<<"cd: "<<s.substr(3)<<": No such file or directory"<<endl;
    }
  }else if(s.substr(0,5)=="echo "){
    cout<<parseString(s.substr(5))<<endl;
  }else if(s.substr(0,5)=="type "){
    string cmnd=parseString(s.substr(5));
    if(cmnd=="echo" || cmnd=="type" || cmnd=="exit" || cmnd=="pwd" || cmnd=="cd"){
        cout<<cmnd<<" is a shell builtin"<<endl;
    }else{
        vector<string> v;
        findExecute(cmnd,v,true);
    }
  }else{
    // stringstream ss(s);
    // string word;
    vector<string> words=tokenizeString(s);
    // Extract words using the >> operator, which naturally splits at spaces
    // while (ss >> word) {
    //     cout<<word<<" ";
    //     words.push_back(word);
    // }
    // cout<<endl;
    for(int i=0;i<words.size();i++){
      cout<<words[i]<<endl;
    }
    bool foundexecutable=findExecute(words[0],words,false);
    if(!foundexecutable)cout<<s<<": command not found"<<endl;
  }
  }
}
