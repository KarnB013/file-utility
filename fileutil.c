#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <string.h>
#include <unistd.h>

//global variables for variables which are defined in main() so that they can be used by external methods
//fname stores file name or extension depending on number of arguments passed
//global_path will store only one path i.e. path to the file which is found first in case of 3 or 5 arguments
//global_path_v2 is an array that stores paths to files with given extension and is used when there are 4 arguments
char *fname, *global_path, global_path_v2[1000][1000];

//itr keeps track of paths stored in global_path_v2
//v_itr keeps track of visited paths/already added files in .tar when 4 arguments are passed
//v2 is used as a flag to separate code in traverse() as we have to find all files with given extension in case of 4 arguments and stop at first occurence in case of 3 or 5 arguments
int itr = 0, v_itr = 0, v2 = 0;

//this function will take a string as input and reverse it
void reverse(char *name){
	int l = strlen(name);
	char *f = name;
	char *e = name + l - 1;
	while(f < e){
		char tmp = *f;
		*f = *e;
		*e = tmp;
		f++, e--;
	}
}

//this is the function that nftw() uses to recursively descend to nested directories to find the file specified by fname
int traverse(const char *path, const struct stat *sb, int flag, struct FTW *ftwbuf){
	//to keep track of files already added to .tar in case of 4 arguments
	char visited[1000][1000];
	//duplicating the root_dir as we need to manipulate it
	char path_fname[200];
	strcpy(path_fname, path);
	//reversing the duplicated root_dir as we need to check if the file name in the end of this path matches with the file we're looking for(i.e. input -> fname)
	reverse(path_fname);
	//as we just need the file name and extension, we're looking until we encounter '/'
	int i = 0;
	//file name with extension will be stored in fname_fnl
	char fname_fnl[200] = {'\0'};
	while(path_fname[i] != '/'){
		strncat(fname_fnl, &path_fname[i], 1);
		i++;
	}
	//the output we get in fname_fnl will be reversed, so, reverse again for correct output
	reverse(fname_fnl);
	//here v2 checks if arguments are 4 or not as we need to stop at first occurence in case of 3 or 5 arguments
	if(!v2){
		//if the the file names match, that means we found the file and store the result in global_path
		if(strncmp(fname_fnl, fname, strlen(fname)) == 0){
			global_path = path;
			printf("%s\n", path);
			//we return 1 as we have found the first occurence and we don't need to look any further
			return 1;
		}
	}
	else{
		//this is the case if number of arguments passed are 4 and we need to make .tar for all files that match given extension and not include files that are already added to .tar
		//as we know fname_fnl has file name with extension, but here, input is just the extension, so, we are going to extract just the extension from fname_fnl in fname_fnl_fnl
		char fname_fnl_fnl[200] = {'\0'};
		int ll = strlen(fname_fnl);
		//we start from end to get just the extension
		for(int i = ll - 1; i >= 0; i--){
			//the moment we see a '.' means we should stop, we found the extension
			if(fname_fnl[i] == '.'){
				break;
			} 
			else{
				//adding the extension character by character to fname_fnl_fnl
				strncat(fname_fnl_fnl, &fname_fnl[i], 1);
			}
		}
		//we need to manually add a '.' as we are not extracting that
		strncat(fname_fnl_fnl, ".", strlen("."));
		//the final result needs to be reversed as we started from the end
		reverse(fname_fnl_fnl);
		//printf("->%s\n%s\n", fname_fnl_fnl, fname);
		//if the extension extracted(fname_fnl_fnl) with given extension(fname), that means the file should be added to .tar
		if(strncmp(fname_fnl_fnl, fname, strlen(fname)) == 0){
			//printf("--->%s\n%s\n", fname_fnl_fnl, fname_fnl);
			int flag = 0;
			//checking if the file already exists in .tar, if yes, we raise a flag
			for(int i = 0; i < v_itr; i++){
				if(strncmp(fname_fnl, visited[i], strlen(visited[i])) == 0){
					flag = 1;
				}
			}
			//if the flag is raised, we ignore the file as it's already added
			if(!flag){
				//add the path to global_path_v2 which will store all paths for files that need to be added to .tar
				strcpy(global_path_v2[itr++], path);
				printf("%s\n", path);
				//as we have added the path of file, we need to mark it as added to .tar
				strcpy(visited[v_itr++], fname_fnl);
			}	
        	}
	}
	return 0;
}

int main(int argc, char *argv[]){
	int k, flags = 0;
	//3 arguments passed, given a root_dir find the file specified
	if(argc == 3){
		//we store the root_dir in root_path
		char *root_path = argv[1];
		//fname stores file name
		fname = argv[2];
		//nftw takes in the root_path, goes to the depth of 20 and we're not including any additional flags
		//nftw returns -1 if it fails, 0 if it exhausted the directory and non-zero if it was explicitly returned from traverse()
		if((k = nftw(root_path, traverse, 20, flags)) != 0){
			if(k == -1){
				printf("Search Unsuccessful\n");
			}
		}
		else{
			//we're explicitly returning 1 from traverse() in case we find the required file
			//if k is not 1, that means file was not found
			if(k != 1){
				printf("Search Unsuccessful\n");
			}
		}
		
	}
	//4 arguments passed, given a root_dir and a storage_dir, find all files that match the extension, add to .tar and store in storage_dir and if storage_dir is not there, create it
	else if(argc == 4){
		//v2 set to 1 to specify this particular case, used in traverse()
		v2 = 1;
		//root_path stores root_dir, new_path stores storage_dir
		char *root_path = argv[1], *new_path = argv[2];
		//fname stores given extension
		fname = argv[3];
		if(fname[0] != '.'){
			printf("Invalid file extension!\n");
		}
		else{
			//nftw takes in the root_path, goes to the depth of 20 and we're not including any additional flags
                	//nftw returns -1 if it fails
			//itr tracks the number of paths stored in global_path_v2 that match the required file extension, if itr == 0 means no matching files with required extension found in root_dir
			k = nftw(root_path, traverse, 20, flags);
			if(k == -1 || itr == 0){
				printf("Search Unsuccessful\n");	
			}
			else{
				//dircheck will store the ls command that will help us check if the storage_dir exists or not
				char dircheck[200] = {'\0'};
				strncat(dircheck, "ls ", strlen("ls "));
				strncat(dircheck, new_path, strlen(new_path));
				//as we don't need the console output, we're redirecting stdout and stderr to /dev/null
				strncat(dircheck, " 1>/dev/null 2>/dev/null", strlen(" 1>/dev/null 2>/dev/null"));
				//system() will invoke the command and if it doesn't return 0 means it failed and we need to create the storage_dir(new_path) 
				if(system(dircheck) != 0){
					//create will store the mkdir command that will help us to create a new required storage_dir
					char create[200] = {'\0'};
					strncat(create, "mkdir -p ", strlen("mkdir -p "));
					strncat(create, new_path, strlen(new_path));
					system(create);			
				}
				//tarf will store the tar command required to archive all matching files to .tar
				char tarf[200] = {'\0'};
				//we're running cf here because we need to create .tar first with -c
				strncat(tarf, "tar cf \"", strlen("tar cf \""));
				//adding storage_dir to specify where to store .tar
				strncat(tarf, new_path, strlen(new_path));
				//specifying name of .tar (i.e. a1.tar)
				strncat(tarf, "/a1.tar\" ", strlen("/a1.tar\" "));
				strncat(tarf, "\"", strlen("\""));
				//sepcifying the path of file to be added to .tar
	                        strncat(tarf, global_path_v2[0], strlen(global_path_v2[0]));
				strncat(tarf, "\"", strlen("\""));
				//redirecting tar output to /dev/null as we don't need console results
				strncat(tarf, " 1>/dev/null 2>/dev/null", strlen(" 1>/dev/null 2>/dev/null"));
				//printf("->%s\n", tarf);
				system(tarf);
				for(int i = 1; i < itr; i++){
					//tarfnl will store tar command to edit existing a1.tar
					char tarfnl[200] = {'\0'};
					//as we created the a1.tar above, we only need to use -r to add to existing a1.tar
					//similar steps below as above
		                        strncat(tarfnl, "tar rf \"", strlen("tar rf \""));
					strncat(tarfnl, new_path, strlen(new_path));
               		                strncat(tarfnl, "/a1.tar\" ", strlen("/a1.tar\" "));
                       		        strncat(tarfnl, "\"", strlen("\""));
                       	        	strncat(tarfnl, global_path_v2[i], strlen(global_path_v2[i]));
                        	       	strncat(tarfnl, "\"", strlen("\""));
					strncat(tarfnl, " 1>/dev/null 2>/dev/null", strlen(" 1>/dev/null 2>/dev/null"));
                        	       	//printf("->%s\n", tarfnl);
					system(tarfnl);
				}
			}
		}
	}
	//5 arguments passed, given a root_dir, storage_dir, options(copy or paste) and a file name, find the file in root_dir, then, copy or paste it to storage_dir as specified 
	else if(argc == 5){
		//root_path stores root_dir, new_path stores storage_dir, opt stores copy or paste option
                char *root_path = argv[1], *new_path = argv[2], *opt = argv[3];
		//fname stores file name
                fname = argv[4];
		//dircheck will store ls to check if root_dir exists or not
		//steps are similar as used in above else condition
		char dircheck[200] = {'\0'};
                strncat(dircheck, "ls ", strlen("ls "));
                strncat(dircheck, root_path, strlen(root_path));
                strncat(dircheck, " 1>/dev/null 2>/dev/null", strlen(" 1>/dev/null 2>/dev/null"));
                if(system(dircheck) != 0){
			printf("Invalid root_dir\n");
                }
		else{
			//dircheck will store ls to check if storage_dir exists or not
			char dircheck[200] = {'\0'};
                	strncat(dircheck, "ls ", strlen("ls "));
                	strncat(dircheck, new_path, strlen(new_path));
                	strncat(dircheck, " 1>/dev/null 2>/dev/null", strlen(" 1>/dev/null 2>/dev/null"));
			//nftw takes in the root_path, goes to the depth of 20 and we're not including any additional flags
	                //nftw returns -1 if it fails
			//as we return 1 explicitly from traverse() when successful, any other k value would mean search failed
                	if((k = nftw(root_path, traverse, 20, flags)) == 1){
				//here, k is 1, therefore we found the file but if dircheck failed, means we found the file but storage_dir doesn't exist
				if(system(dircheck) != 0){
					printf("Search Successful : Invalid storage_dir\n");
				}
				else{
					//if option was specified to copy the file
					if(strcmp(opt, "-cp") == 0){
						//we use execvp() to replace the process with cp command to copy it to given storage_dir
                               			char *copy = "cp";  
                       			        char *args[] = {"cp", global_path, new_path, NULL};
						printf("Search Successful\nFile copied to the storage_dir\n");
						int a = execvp(copy, args);
						return 0;
                        		}
					//if option was specified to move the file
                       			else if(strcmp(opt, "-mv") == 0){
						//we use execvp() to replace the process with mv command to move it to given storage_dir
                       				char *move = "mv";
                               			char *args[] = {"mv", global_path, new_path, NULL};
						printf("Search Successful\nFile moved to the storage_dir\n");
                               			int a = execvp(move, args);
                                		return 0;	        
                        		}
                        		else{
						//in case the user enters something other than -cp or -mv in options
                               			printf("Specify correctly if you want to copy or move the file");
                       			}
				}
               		}
               		else{ 
				//k is not 1
                        	printf("Search Unsuccessful\n");	
                	}
		}
		
	}
	else{
		printf("Invalid number of arguments\n");
	}
	return 0;
}
