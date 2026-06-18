#include<bits/stdc++.h>
using namespace std;

int main() {
  for(;;){
  cout << "$ ";
  string s;
  getline(cin, s);
  if(s=="exit")break;
  if(s.lenth()>=5 && s.substr(0,5)=="echo "){
    cout<<s.substr(5)<<endl;
  }else{
    cout<<s<<": command not found"<<endl;
  }
  }
}
