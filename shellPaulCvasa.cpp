//RUN using g++ shellPaulCvasa.cpp -lreadline

#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <chrono>
#include <time.h>
#include <libgen.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <semaphore.h>

using namespace std;

int k=0,pipek=0; //used for finding if a command has args
int nrpipe=0; //number of pipes

void getHelp()
{
	cout<<"\nCurrent implemented commands: \n~help\n~dirname\n~ls (-l, -s, -a, -F)\n~tac (-b, -s)\n~version\n~exit\n + all the default commands.";
}

void getVersion()
{
	cout<<"\n~~~Shell Version: 1.0~~~\n~~~Author: Paul Cvasa~~~\n~~~©2020 Paul Cvasa. All rights reserved~~~";
}

int inputCommands(char* str)
{
	char* buff;
	buff = readline("$ ");
	if(strlen(buff))
	{
		add_history(buff);
		strcpy(str,buff);
		return 0;
	}
	else
		return 1;
}

void runCommandExec(char** command)
{
	pid_t pid = fork();
	if(pid == 1)
	{
		cout<<"\nERROR: Fork failed.";
		return;
	}
	else if(pid == 0)
	{
		if(execvp(command[0], command)<0)
			cout<<"\nERROR: Command not executed.";
		exit(0);
	}
	else
	{
		wait(NULL);
		return;
	}
}

void get_dir()
{
	char currentDir[2048];
	int dirSize = sizeof(currentDir);
	getcwd(currentDir, dirSize);
	cout << "\nCurrent directory: " << currentDir << "~";
}

/*typedef int Pipe[2];

void execPipeCommand(int nr, char** commands, Pipe output);

void exec_nthCommand(int nr, char** commands)
{
	if(nr > 1)
	{
		pid_t pid;
		Pipe input;
		if(pipe(input)!=0)
			exit(1);
		if((pid = fork())<0)
			exit(2);
		if(pid==0)
			execPipeCommand(nr-1, commands, input);
		dup2(input[0],0);
		close(input[0]);
		close(input[1]);
	}
	execvp(commands[nr-1], commands);
}

void execPipeCommand(int nr, char** commands, Pipe output)
{
	dup2(output[1],1);
	close(output[0]);
	close(output[1]);
	exec_nthCommand(nr, commands);
}
*/
void runPipedCommands(char** command, char** pipedcommand)
{
	int pipe_RW[2]; //pipe for READ/WRITE
	int pid1, pid2;

	if(pipe(pipe_RW)<0)
	{
		cout << "\nERROR: Pipe failed to start.";
		return;
	}
	pid1 = fork();
	if(pid1 == 0)
	{
		dup2(pipe_RW[1], STDOUT_FILENO);
		close(pipe_RW[0]);
		close(pipe_RW[1]);
		if(execvp(command[0], command)<0)
		{
			cout<<"\nERROR: Command 1 is incorrect/failed to execute.";
			exit(0);
		}
	}
	
	pid2 = fork();
	if(pid2<0)
	{
		cout<<"\nERROR: Fork failed.";
		return;
	}

	if(pid2 == 0)
	{
		dup2(pipe_RW[0], STDIN_FILENO);
		close(pipe_RW[0]);
		close(pipe_RW[1]);
		if(execvp(pipedcommand[0], pipedcommand)<0)
		{
			cout<<"\nERROR: Command 2 is incorrect/failed to execute.";
			exit(0);
		}
	}
	close(pipe_RW[0]);
	close(pipe_RW[1]);

	waitpid(pid1,NULL,0);
	waitpid(pid2,NULL,0);
}


int findPipe(char* comm, char** commpiped)
{
	for(int i=0;i<2;i++)
	{
		commpiped[i]=strsep(&comm, "|");
		nrpipe++;
		if(commpiped[i]==NULL)
			i=3;
	}
	if(commpiped[1]==NULL)
		return 0;
	return 1;
}

void findWords(char* comm, char** output)
{
	for(int i=0;i<100;i++)
	{
		output[i]=strsep(&comm, " ");
		k++;
		if(output[i]==NULL)
			break;
		if(strlen(output[i])==0)
			i--;
	}

}

void ls()
{
	DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

	if (!(dir = opendir(".")))
        return;
	cout<<"\n";
	while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type != DT_DIR) 
		{
            stat(entry->d_name, &fileStat);
			//cout<<"\n";
            cout<<" "<< entry->d_name;
        }
    }
    closedir(dir);
}

void ls_a()
{
	struct dirent **dirlist;
	int n = scandir(".", &dirlist, NULL, alphasort);

	if(n<0)
	{
		cout<<"ERROR: scandir";
		return;
	}
	else
	{
		cout<<"\n";
		while(n--)
		{
			cout<<dirlist[n]->d_name<<" ";
			free(dirlist[n]);
		}
		free(dirlist);
	}
}

void getUser(uid_t uid)
{
	if (getpwuid(uid)!=NULL)
	{
		cout<<" "<<getpwuid(uid)->pw_name;
		return;
	}
	else
	{
		cout<<" "<<uid;
		return;
	}
	
}

void getGroup(gid_t gid)
{
	if(getgrgid(gid)!=NULL)
	{
		cout<<" "<<getgrgid(gid)->gr_name;
		return;
	}
	else
	{
		cout<<" "<<gid;
		return;
	}
}

void ls_l()
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    if (!(dir = opendir(".")))
        return;

    while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type == DT_DIR) 
		{
            char path[1024];
			// this was used for recursive search \/
            //if (filePermissions(entry->d_name))
                //cout<<"Error getting file information\n");
            //if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                //continue;
            //snprintf(path, sizeof(path), "%s/%s", ".", entry->d_name);
            //cout<<"["<<entry->d_name<<"]\n";
            //ls_l(path, indent + 2);
            
        } else 
		{
            stat(entry->d_name, &fileStat);
			cout<<"\n";
            S_ISDIR(fileStat.st_mode) ? cout<<"d" : cout<<"-";
			//permissions
            (fileStat.st_mode & S_IRUSR) ? cout<<"r" : cout<<"-";
            (fileStat.st_mode & S_IWUSR) ? cout<<"w" : cout<<"-";
            (fileStat.st_mode & S_IXUSR) ? cout<<"x" : cout<<"-";
            (fileStat.st_mode & S_IRGRP) ? cout<<"r" : cout<<"-";
            (fileStat.st_mode & S_IWGRP) ? cout<<"w" : cout<<"-";
            (fileStat.st_mode & S_IXGRP) ? cout<<"x" : cout<<"-";
            (fileStat.st_mode & S_IROTH) ? cout<<"r" : cout<<"-";
            (fileStat.st_mode & S_IWOTH) ? cout<<"w" : cout<<"-";
            (fileStat.st_mode & S_IXOTH) ? cout<<"x" : cout<<"-";
            cout<<" "<<fileStat.st_nlink;
			getUser(fileStat.st_uid);
			getGroup(fileStat.st_gid);
            cout<<" "<<fileStat.st_size;
            //cout<<" "<<fileStat.st_blocks;
			string atime = asctime(gmtime(&fileStat.st_ctime));
			atime.pop_back();
            cout<<" "<<atime;
            cout<<" "<< entry->d_name;
            //cout<<"\n";
            
        }
    }
    closedir(dir);
}

void ls_s()
{
	DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

	if (!(dir = opendir(".")))
        return;

	while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type != DT_DIR) 
		{
            stat(entry->d_name, &fileStat);
			cout<<"\n";
            cout<<" "<<fileStat.st_blocks;
            cout<<" "<< entry->d_name;
        }
    }
    closedir(dir);
}

void ls_F()
{
	DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

	if (!(dir = opendir(".")))
        return;

	while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type != DT_DIR) 
		{
            stat(entry->d_name, &fileStat);
            cout<<" "<< entry->d_name;
			if(S_ISDIR(fileStat.st_mode))
				cout<<"/";
			else if(S_ISLNK(fileStat.st_mode))
				cout<<"@";
			else if(S_ISSOCK(fileStat.st_mode))
				cout<<"=";
			else if(S_ISFIFO(fileStat.st_mode))
				cout<<"|";
			else if(S_ISREG(fileStat.st_mode))
				cout<<"*";
        }
    }
    closedir(dir);
}

char *dirname (char *path)
{
	char *last_slash;

  	if(path!=NULL)
		last_slash=strrchr(path,'/');
  	else
		last_slash=NULL;

  	if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
	{
    	char *runpath;
    	for (runpath = last_slash; runpath != path; --runpath)
    	if (runpath[-1] != '/')
        	break;
    }
  	if (last_slash != NULL)
    {
    	char *runpath;
      	for (runpath = last_slash; runpath != path; --runpath)
        if (runpath[-1] != '/')
        	break;
      	if (runpath == path)
        {
        	if (last_slash == path + 1)
            	++last_slash;
        	else
            	last_slash = path + 1;
        }
      	else
        	last_slash = runpath;
      	last_slash[0] = '\0';
    }
  	else
	{
		static const char dot[] = ".";
    	path = (char *) dot;
	}
  	return path;
}

void tac(char* filename)
{
	ifstream input(filename);

	if(!(input.is_open()))
	{
		//cout<<"\nERROR: Failed to open file";
		return;
	}
	else
	{
		vector<string> lines;
		string l;
		while(getline(input,l))
			lines.push_back(l);
		cout<<"\n";
		input.get();
		for(int i=lines.size()-1;i>=0;i--)
			cout<<lines[i];
	}
	input.close();
}

int handleCommands(char * command[100], int k)
{
	if(strcmp(command[0], "help")==0)
	{
		if(k>3)
		{
			if(strcmp(command[1],">")==0)
			{
				int defaultSTDOUT = dup(STDOUT_FILENO);
				int output = open(command[2], O_CREAT | O_WRONLY);
				dup2(output, STDOUT_FILENO);
				getHelp();
				dup2(defaultSTDOUT, STDOUT_FILENO);
				close(defaultSTDOUT);
				return 1;
			}
		}
		getHelp();
		return 1;
	}
	else if(strcmp(command[0], "ls")==0 && k==2)
	{
		ls();
		return 1;
	}
	else if((strcmp(command[0], "ls")==0) && (strcmp(command[1],">")==0))
	{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[2], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			ls();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
	}
	else if((strcmp(command[0],"ls")==0) && (strcmp(command[1],"-l")==0))
	{
		if(k>3 && (strcmp(command[2],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[3], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			ls_l();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		ls_l();
		return 1;
	}
	else if((strcmp(command[0],"ls")==0) && (strcmp(command[1],"-s")==0))
	{
		if(k>3 && (strcmp(command[2],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[3], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			ls_s();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		ls_s();
		return 1;
	}
	else if((strcmp(command[0],"ls")==0) && (strcmp(command[1],"-a")==0))
	{
		if(k>3 && (strcmp(command[2],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[3], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			ls_a();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		ls_a();
		return 1;
	}
	else if((strcmp(command[0],"ls")==0) && (strcmp(command[1],"-F")==0))
	{
		if(k>3 && (strcmp(command[2],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[3], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			ls_F();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		ls_F();
		return 1;
	}
	else if((strcmp(command[0],"tac")==0))
	{
		if(strcmp(command[1],"-b")!=0)
		{
			if(k==3)
			{
				tac(command[1]);
				tac(command[2]);
				return 1;
			}
			if(k==4)
			{
				tac(command[1]);
				tac(command[2]);
				tac(command[3]);
				return 1;
			}
			tac(command[1]);
			return 1;
		}
		else if(strcmp(command[1],"-b")==0)
		{
			if(k==4)
			{
				cout<<"\n";
				tac(command[2]);
				tac(command[3]);
				cout<<"\b";
				return 1;
			}
			if(k==5)
			{
				cout<<"\n";
				tac(command[2]);
				tac(command[3]);
				tac(command[4]);
				cout<<"\b";
				return 1;
			}
			cout<<"\n";
			tac(command[2]);
			cout<<"\b";
			return 1;
		}
	}
	else if(strcmp(command[0],"dirname")==0)
	{
		if(k>3 && (strcmp(command[1],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[2], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			cout<<"\n"<<dirname(command[1]);
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		cout<<"\n"<<dirname(command[1]);
		return 1;
	}
	else if(strcmp(command[0],"version")==0)
	{
		if(k>3 && (strcmp(command[1],">")==0))
		{
			int defaultSTDOUT = dup(STDOUT_FILENO);
			int output = open(command[2], O_CREAT | O_WRONLY);
			dup2(output, STDOUT_FILENO);
			getVersion();
			dup2(defaultSTDOUT, STDOUT_FILENO);
			close(defaultSTDOUT);
			return 1;
		}
		getVersion();
		return 1;
	}
	else if(strcmp(command[0], "exit")==0)
		exit(0);
	return 0;
}


void redirection(char *filename, char** command)
{
	int defaultSTDOUT = dup(STDOUT_FILENO);
	int output = open(filename, O_CREAT | O_WRONLY);
	dup2(output, STDOUT_FILENO);
	runCommandExec(command);
	dup2(defaultSTDOUT, STDOUT_FILENO);
	close(defaultSTDOUT);
}


void TCPServer()
{
	char data[512] = "\nConnected to SERVER";

	int socketServer = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(9005);
	server.sin_addr.s_addr=INADDR_ANY;

	bind(socketServer, (struct sockaddr*)&server, sizeof(server));
	listen(socketServer, 5);

	int clientSocket = accept(socketServer, NULL, NULL);
	send(clientSocket, data, sizeof(data), 0);

	close(socketServer);
	return;
}


int readCommand(char* comm, char* output[100], char* outputpipe[100])
{
	char* commpiped[2];
	int p;
	int ok1=0,ok2=0;

	p=findPipe(comm, commpiped);
	if(p)
	{
		findWords(commpiped[0], output);
		ok1=handleCommands(output,k);
		pipek=k-1;
		findWords(commpiped[1], outputpipe);
		k=0;
		ok2=handleCommands(outputpipe,pipek);

		if(ok1 && ok2)
			return 0;
		else
			return 1+p;
	}
	else
	{
		findWords(comm,output);
		ok1=handleCommands(output,k);
		if(ok1)
			return 0;
		else
			return 1+p;
		
	}
	

	//cout<<"\nOUTPUT: "<<*output;
	 
}


int splitCommands(char input[], char * commands[])
{
	int nr=0;
	char *p;
	p=strtok(input," |");
	while(p!=NULL)
	{
		//strcpy(commands[nr],p);
		//cout<<"\np: "<<p;
		commands[nr] = p;
		//cout<<"\ncom: "<<commands[nr];
		p=strtok(NULL,"|");
		nr++;
		//if(handleCommands(commands[nr]))
	}
	return nr;
}

int main()
{
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SHELL STARTED~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	cout << "\t\tCurrent user: " << getenv("USER") << "\n";

	int state = 1;
	char input[100], *commands[100];
	char* commandsPiped[100];
	int read;
	TCPServer();
	while(state)
	{
		k=0;
		pipek=0;
		get_dir();
		if(inputCommands(input))
			continue;
		read=readCommand(input,commands, commandsPiped);
		//read = splitCommands(input, commands);
		//cout<<"\nINPUT: "<<input;
		//cout<<"\nCOMMANDS: "<<*commands;
		//cout<<"\nNr of commands: "<<read;

		//exec_nthCommand(read, commands);
		//if(read ==-1)
		//{

		//}
		//else
		//runPipedCommands(commands, commandsPiped);
		//if(read == 0)
		//{
			//if(nrpipe = 1)
				//runPipedCommands(commands, commandsPiped);
		//}
		
		 if(read == 1)
		{
			runCommandExec(commands);
		}
		else if(read == 2)
		{
			runPipedCommands(commands, commandsPiped);
		}
		
			

	}

	return 0;
}