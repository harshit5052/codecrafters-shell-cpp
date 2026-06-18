#include<bits/stdc++.h>
using namespace std;

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
        cout<<cmnd<<": not found"<<endl;
    }
  }else{
    cout<<s<<": command not found"<<endl;
  }
  }
}
