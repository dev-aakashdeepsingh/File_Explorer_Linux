#include "headers.h"

#define CTRL_KEY(k) ((k) & 0x1f)
using namespace std;
int pointy;
bool found_f=false;
vector<string> info;
string path;
stack<string> forward1;
stack<string> back1;
struct termios orig_termios;


string tild(string p){
  int loc=p.find_first_of('~');
  if(loc==string::npos){
    return p;
  }
  string idd=getlogin();
  string temp="/home/"+idd;
  p.replace(loc,loc+1,temp);
  /*
  if ((pwd = getpwuid(buf.st_uid)) != NULL){
    string a=pwd->pw_name;
    string temp="/home/"+a;
    cout<<"helloo "<<a<<"\r\n";
    p.replace(loc-1,loc,temp);
    return p;
  }
  else{
    string a=to_string(buf.st_uid);
    string temp="/home/"+a;
    cout<<"hell "<<a<<"\r\n";
    p.replace(loc,loc+1,temp);
    return p;
  }
  */
  return p;
}

string relative(string p){
    if(p[0]=='/'){
      return p;
    }
    string temp=path+"/"+p;
    return temp;  
}


void die(const char *s) {
  perror(s);
  exit(1);
}



char readinput(char c){
    if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': return 'w';
        case 'B': return 's';
        case 'C': return 'd';
        case 'D': return 'a';
      }
    }
    return '\x1b';
  } 
  else{
    return c;
  }
}



char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}


void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}


void disableRawMode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)==-1) die("tcsetattr");
}


void rawMode() {
  if(tcgetattr(STDIN_FILENO, &orig_termios)==-1) die("tcgetattr");
  atexit(disableRawMode);  // disabling raw mode line at exit
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(ICRNL | IXON | INPCK | BRKINT | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1) die("tcsetattr");
}


bool check_file(){
    if(info[pointy*6][0]=='d'){
      return true;
    }
    return false;
}


void print_listi(int offset=0){
  //cout<<"\033[2J";
  editorRefreshScreen();
  int n=info.size();
  int m=n/6;
  int i=0;
  m=min(m,30);
  if(pointy>30){
  offset=pointy-29;
  }
  i+=offset;
  for(;i<m+offset;i++){
    if(i!=pointy){
      cout<<"      ";
    }
    else if(i==pointy){    //total=pointy
      cout<<">>    ";
    }
    for(int j=0;j<6;j++){
        if(j==3){
          int length=info[i*6+j].size();
          cout<<info[i*6+j];
          length=12-length;
          for(int k=0;k<length;k++){
            cout<<" ";
          }
        }
        else{
        cout<<info[i*6+j]<<"   ";
        }
    }
    cout<<"\r\n";
    //total++;
    //if(error)
    //cout<<"ERRORRRWRWERWR"<<"\r\n";
  }
}


void listi(string pathe=path){
  info.clear();
  struct dirent *entry;
  struct stat buf;
  struct passwd *pwd;
  struct group *grp;
  DIR *dir;
  struct tm *tm;
  char datestring[256];

  if((dir = opendir(pathe.c_str())) == NULL){
    cout<<"path inside opendir"<<pathe<<"\r\n";
    cout<<pathe.size()<<"\r\n";
    perror("opendir() error");
  }
  else {
    while ((entry = readdir(dir)) != NULL){ 
      //&& strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
    //if (stat(entry->d_name, &buf) == -1){
    //    continue;
    //}
    string temp=pathe;
    if(pathe=="/"){
      temp+=entry->d_name;
    }
    else{
    temp+="/";
    temp+=entry->d_name;
    }
    //cout<<temp<<" ";
    if(stat(&temp[0],&buf) == -1) continue;  //error=true
    
    string perm="";
    perm+=(S_ISDIR(buf.st_mode) ? "d" : "-");
    perm+=((buf.st_mode & S_IRUSR) ? "r" : "-");
	  perm+=((buf.st_mode & S_IWUSR) ? "w" : "-");
	  perm+=((buf.st_mode & S_IXUSR) ? "x" : "-");
	  perm+=((buf.st_mode & S_IRGRP) ? "r" : "-");
	  perm+=((buf.st_mode & S_IWGRP) ? "w" : "-");
	  perm+=((buf.st_mode & S_IXGRP) ? "x" : "-");
    perm+=((buf.st_mode & S_IROTH) ? "r" : "-");
	  perm+=((buf.st_mode & S_IWOTH) ? "w" : "-");
	  perm+=((buf.st_mode & S_IXOTH) ? "x" : "-");
    //printf("  %s  ", entry->d_name);
    //cout<<perm<<" ";
    info.push_back(perm);
     if ((pwd = getpwuid(buf.st_uid)) != NULL){
        //printf(" %-8.8s  ", pwd->pw_name);
        info.push_back(pwd->pw_name);
     }
    else{
        //printf(" %-8d  ", buf.st_uid);
        info.push_back(to_string(buf.st_uid));
    }
      //printf(" %o\n",buf.st_mode);
     if ((grp = getgrgid(buf.st_gid)) != NULL){
        //printf(" %-8.8s  ", grp->gr_name);
        info.push_back(grp->gr_name);
     }
    else{
        //printf(" %-8d  ", buf.st_gid);
        info.push_back(to_string(buf.st_gid));
    }       
    //printf(" %9jd  ", (intmax_t)buf.st_size);
    info.push_back(to_string((intmax_t)buf.st_size));
    tm = localtime(&buf.st_mtime);
    /* Get localized date string. */
    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
    info.push_back(datestring);
    info.push_back(entry->d_name);
    //printf(" %s %s\n", datestring, entry->d_name); 
    }
    closedir(dir);
  }
}



void searchee(string namee,string pathe=path){
  //cout<<"hii\r\n"<<"name="<<namee<<"\r\n";
  pathe=tild(pathe);
  pathe=relative(pathe);
  listi(pathe);
  vector<string> temp;
  int m=info.size();
  m=m/6;
  for(int i=0;i<m;i++){
    cout<<info[i*6+5]<<"\r\n";
    if(info[i*6+5].compare(namee)){
      found_f=true;
      return;
    }
    if((info[i*6][0])=='d'){
      string temp1=pathe;
      temp1+="/";
      temp1+=info[i*6+5];
      temp.push_back(temp1);
    }
  }
  while(temp.empty()==false && found_f==false){
    string new_p=temp.back();
    temp.pop_back();
    listi(new_p);
    int m=info.size();
    m=m/6;
    for(int i=0;i<m;i++){
      if(info[i*6+5]==namee){
        found_f=true;
        return;
      }
      if(info[i*6][0]=='d' && info[i*6+5]!="." && info[i*6+5]!=".."){
        string temp1=new_p;
        temp1+="/";
        temp1+=info[i*6+5];

        temp.push_back(temp1);
      }
    }
  }
}

void createFile(string file1, string desti)
{
      desti=tild(desti);
      desti=relative(desti);
      string desti_file=desti+"/"+file1;
      if(creat(desti_file.c_str(),0600)==-1)
            printf("couldn't create file");
        else 
            cout<<"Operation performed succesfully\n\r";
      
}

void createDir(string direc, string desti)
{
        desti=tild(desti);
        desti=relative(desti);
        string dest=desti+"/"+direc;
        if(mkdir(dest.c_str(),0755)==-1)
            printf("couldn't create directory");
        else 
            cout<<"Operation performed successfully\r\n";
    
}

void renamee(string a, string b){
  a=tild(a);
  a=relative(a);
  b=tild(b);
  b=relative(b);
  //cout<<"a="<<a<<" b="<<b;
  rename(a.c_str(),b.c_str());
  cout<<"Operation done\r\n";
}

void gotoo(string namee){
  pointy=0;
  
  int i=0;
  string temp;
  int length=namee.size();
  while(i<length){
    if(!isspace(namee[i])){
      temp+=namee[i];
    }
    i++;
  }
  temp=tild(temp);
  temp=relative(temp);
  path=temp;
  //cout<<"path="<<path<<" path.size()="<<path.size()<<"\r\n";
  listi();
  //cout<<"hheeehhh\r\n";
  print_listi();
  cout<<"Command Mode:  \r\n";
}

void copyFile(string file_name, string destin){
  /*
  char ch[1024];
  int one;
  int two;
  if(one=open(file_name.c_str(), O_RDONLY)==-1){
    cout<<"\rFile given has an error\n";
    close(one);
    return;
  }
  int count;
  int last=file_name.find_last_of("/");
  string temp=file_name.substr(last+1);
  cout<<"temp="<<temp<<"\r\n";
  if((two=open((destin+'/'+temp).c_str(),O_CREAT|S_IRUSR|O_WRONLY|S_IWUSR))==-1){
        cout<<"file already exists";
        return;
   }
   cout<<"hi";
   while ((count = read(one, ch, sizeof(ch))) > 0){
	      write(two, ch, count);
   }
  
		struct stat st;
		stat(file_name.c_str(), &st);
		chmod((destin+'/'+temp).c_str(), st.st_mode);
    
    cout<<"helllo\r\n";     
    close(one);
    close(two);
    return;
    */
        file_name=tild(file_name);
        file_name=relative(file_name);
        destin=tild(destin);
        destin=relative(destin);
        char block[1024];
        int in , out, nread; in = open(file_name.c_str(), O_RDONLY);
        int last=file_name.find_last_of("/");
        string temp=file_name.substr(last+1);
        out = open((destin + '/' + temp).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        while ((nread = read( in , block, sizeof(block))) > 0) {
            write(out, block, nread);
        }
        close(in);
        close(out);
}

void deleteFile(string file_name){
  file_name=tild(file_name);
  file_name=relative(file_name);
  struct stat buf;
  if(stat(file_name.c_str(), &buf) == -1 || S_ISDIR(buf.st_mode) || remove(file_name.c_str()) != 0) {
    cout<<"Error deleting file\r\n";
    return;
  }
  cout<<"Done\r\n";
  return;//remove returns 0 on success

}

void moveFile(string file_name,string destin){
  file_name=tild(file_name);
  file_name=relative(file_name);
  destin=tild(destin);
  destin=relative(destin);
  copyFile(file_name,destin);
  deleteFile(file_name);
}
/*
void copy_dir(string s,string d){
  vector<string> temp1;
  s=tild(s);
  s=relative(s);
  d=tild(d);
  d=relative(d);
  string t1;
  string t2;
  listi(s);
  int length=info.size();
  length=length/6;
  int loc=s.find_last_of("/");
  if(loc==string::npos){
      //cout<<"heerre";
      //createDir(s,d);
  }
  else{
    int leni=s.size();
    string namee=s.substr(loc+1,leni-1);
    string tempo;
    int j=0;
    int length2=d.size();
    while(j<length2-1){
      tempo+=d[j];
      j++;
    }
    cout<<tempo<<"\r\n";
    createDir(namee,tempo);

  }
  */
  /*
  int i=0;
  for(;i<length;i++){
    if((info[i*6][0])=='d'){
      string temp2=s;
      string temp3=d;
      temp2+="/";
      temp2+=info[i*6+5];
      temp3+="/";
      temp3+=info[i*6+5];
      temp1.push_back(temp2);
      temp1.push_back(temp3);
      createDir(info[i*6+5],d);
    }
    else{
      string temp9=s;
      temp9+=info[i*6+5];
      copyFile(temp9,d);
    }
  }
  */
  /*
  while(temp1.empty()==false){
    string too=temp1.back();
    temp1.pop_back();
    string fro=temp1.back();
    temp1.pop_back();
    listi(fro);
    int leni=info.size();
    leni=leni/6;
    int i=0;
    for(;i<leni;i++){
      if(info[i*6][0]=='d'){
        string temp2=fro;
        string temp3=too;
        temp2+="/";
        temp2+=info[i*6+5];
        temp3+="/";
        temp3+=info[i*6+5];
        temp1.push_back(temp2);
        temp1.push_back(temp3);
        createDir(info[i*6+5],too);
      }
      else{
        string temp9;
        temp9=fro;
        temp9+=info[i*6+5];
        copyFile(temp9,too);
      }
    }
    
  }
 
}
 */
void command_mode(){
  //disableRawMode();
  cout<<"Command Mode:  \r\n";
  string inp;
  while(1){
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    inp+=c;
    if(c=='\x1b'){
      //enable normal mode
      break;
    }
    else if(c=='\r'){
      //cout<<"inp="<<inp<<"\r";
      int i=0;
      string func;
      int length=inp.size();
      while(!isspace(inp[i])){
        func+=inp[i];
        i++;
      }
      //cout<<func<<"\r\n";

      if(func=="quit"){
        exit(1);
      }


      if(func=="search"){
        i++;
        string namee;
        while(i<length-1){
          namee+=inp[i];
          i++;
        }
        //cout<<namee;
        found_f=false;
        searchee(namee);
        if(found_f==true){
          cout<<"File is present\n";
        }
        else{
          cout<<"File not found\n";
        }
        func.clear();
        inp.clear();
      }


      if(func=="goto"){
        i++;
        string namee;
        while(i<length-1){
          namee+=inp[i];
          i++;
        }
        gotoo(namee);
        func.clear();
        inp.clear();
      }

      if(func=="rename"){
        i++;
        string a;
        string b;
        while(!isspace(inp[i])){
          a+=inp[i];
          i++;
        }
        i++;
        int loc=a.find_last_of('/');
        string c=a.substr(0,loc+1);
        b+=c;
        while(i<length-1){
          b+=inp[i];
          i++;
        }
        pointy=0;
        renamee(a,b);
        func.clear();
        inp.clear();
      }

      if(func=="create_file"){
        i++;
        string file_name;
        while(!isspace(inp[i])){
          file_name+=inp[i];
          i++;
        }
        i++;
        string destin;
        int length=inp.size();
        //cout<<file_name<<"   "<<length;
        while(i<length-1){
          destin+=inp[i];
          i++;
        }
        //cout<<file_name<<"   "<<destin;
        createFile(file_name,destin);
      }
      
      if(func=="create_dir"){
        i++;
        string file_name;
        while(!isspace(inp[i])){
          file_name+=inp[i];
          i++;
        }
        i++;
        string destin;
        int length=inp.size();
        while(i<length-1){
          destin+=inp[i];
          i++;
        }
        createDir(file_name,destin);
      }

      if(func=="copy_file"){
        i++;
        string file_name;
        while(!isspace(inp[i])){
          file_name+=inp[i];
          i++;
        }
        //i++;
        int length=inp.size();
        int j=length-2;
        string destin;
        //cout<<j<<" "<<inp[j]<<"\r\n";
        while(!(isspace(inp[j]))&&j>i){
          destin+=inp[j];
          j--;
        }
        reverse(destin.begin(), destin.end());
        if(i>=j){
          //cout<<"File name="<<file_name<<" destin="<<destin<<"\r\n";
          copyFile(file_name,destin);
          func.clear();
          inp.clear();
          continue;
        }
        else{
          //file_name.clear();
          //cout<<"File name="<<file_name<<" destin="<<destin<<"\r\n";
          while(i<j){
            if(isspace(inp[i])){
              copyFile(file_name,destin);
              file_name.clear();
              i++;
            }
            file_name+=inp[i];
            i++;
          }
        }
   
      }

      if(func=="delete_file"){
        i++;
        string file_name;
        int length=inp.size();
        while(i<length-1){
          file_name+=inp[i];
          i++;
        }
        deleteFile(file_name);
      }

      if(func=="move_file"){
        i++;
        string file_name;
        while(!isspace(inp[i])){
          file_name+=inp[i];
          i++;
        }
        i++;
        string destin;
        int length=inp.size()-1;
        while(i<length){
          destin+=inp[i];
          i++;
        }
        //cout<<"file_name="<<file_name<<" destin="<<destin;
        moveFile(file_name,destin);
      }
/*
      if(func=="copy_dir"){
        i++;
        string file_name;
        while (!isspace(inp[i]))
        {
          file_name+=inp[i];
          i++;
        }
        i++;
        string destin;
        int legnth=inp.size()-1;
        while(i<length){
          destin+=inp[i];
          i++;
        }
        cout<<"directory_loc "<<file_name<<" destin "<<destin;
        copy_dir(file_name,destin);

      }
*/
      func.clear();
      inp.clear();
    }
    else if(c==127){
      inp=inp.substr(0,inp.size()-2);
      print_listi();
      cout<<"Command Mode:\r\n";
      cout<<inp<<"\r\n";
    }
    else{
      print_listi();
      cout<<"Command Mode:\r\n";
      cout<<inp<<"\r\n";
    }
  }
}


void normal(){
  rawMode();
  pointy=0;
  listi("/");
  print_listi();
  while(1){
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (c == 'q') break;
    c=readinput(c);

    if(c=='s'){
      //cout<<"helllooo";
      int m=info.size();
      m=m/6;
      if(pointy<m-1){
        pointy+=1;
        int offset=0;
        if(pointy>29){
        int offset=pointy%29;
        }
        print_listi(offset);
      }
    }

    if(c=='w'){
      int m=info.size();
      m=m/6;
      if(pointy>0){
        pointy-=1;
        int offset=0;
        if(pointy>29){
        offset=pointy%29;
        }
        print_listi(offset);
      }
    }

    if(c=='h'){
      editorRefreshScreen();
      pointy=0;
      string idd=getlogin();
      path="/home/"+idd;
      listi(path);
      print_listi();
    }

    if(c=='\r'){
        string temp_path;
        if(path=="/"){
        temp_path=path+info[pointy*6+5];
        }
        else{
          temp_path=path+"/";
          temp_path+=info[pointy*6+5];
        }
        //cout<<pointy<<" ";
        //cout<<temp_path<<"\n";
        int a=check_file();
        if(a==false){
          //cout<<"this is a file";
          if(fork()==0){
            execlp("xdg-open","xdg-open",temp_path.c_str(),NULL);
            exit(0);
          }
        }
        else{
          if(info[pointy*6+5]==".."){
          pointy=0;
          int loc=path.find_last_of('/');
          path=path.substr(0,loc);
          if(path.empty()==true){
            path="/";
          }
          listi(path);
          print_listi();
          while(forward1.empty()==false){
          forward1.pop();
          }
          back1.push(path);
          continue;
          }
          if(info[pointy*6+5]=="."){
            path="/";
            listi(path);
            pointy=0;
            print_listi();
            back1.push(path);
            continue;
          }
          if(path=="/"){
            path+=info[pointy*6+5];
          }
          else{
            path+="/";
            path+=info[pointy*6+5];
          }
          back1.push(path);
          //cout<<pointy<<"\n";
          //cout<<path;
          listi(path);
          pointy=0;
          print_listi();
          //cout<<"this is a directory";
        }
    }

    if(c==127){
      pointy=0;
      int i=0;
      int length=path.size();
      for(int j=0;j<length;j++){
        if(path[j]=='/'){
          i++;
        }
      }
      if(i>1){
        int loc=path.find_last_of('/');
        path=path.substr(0,loc);
      }
      else{
        int loc=path.find_last_of('/');
        path=path.substr(0,loc+1);
      }
      //cout<<path;
      /*
      while(forward1.empty()==false){
        forward1.pop();
      }
      */
      back1.push(path);
      listi(path);
      print_listi();
    }

    if(c=='a'){
      if(back1.empty()==false){
      string temp=back1.top();
      back1.pop();
      forward1.push(temp);
      path=temp;
      //cout<<path<<"\n";
      listi(path);
      pointy=0;
      print_listi();
      }
    }

    if(c=='d'){
      if(forward1.empty()==false){
        string temp=forward1.top();
        forward1.pop();
        back1.push(temp);
        //cout<<path<<"\n";
        path=temp;
        listi(path);
        pointy=0;
        print_listi();
      }
    }
    
    if(c==58){    //change to : later before submitting
      command_mode();
      print_listi();
    }

  }
}


int main() {
 // rawMode();

  path="/";
  back1.push(path);
  
  normal();
  return 0;
}

