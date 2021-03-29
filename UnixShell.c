#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//parse the user input into usable code, looking for deliminatino characters
char** parser(char* input, char* args[]){
    //space and tab will be the deliminator characters
    const char deliminators[2] = {' ', '\n'};
    char* token = strtok(input,deliminators); //initial strtok call using the input and deliminators 
    args[0] = token;
    int commands = 0;  // number of commands, used as an iterator for args



    char** hold = malloc(2 * sizeof(char*)); //hold the token and input/output 
    hold[0]=""; //to be replaced with "input" or "output" if we receive > or <
    //hold[1] will hold onto the token once parsed 

    //loop for parsing the token 
    while(true){
        token = strtok(NULL, deliminators);//continue strtok call until reduced to null 
        if(token == NULL) {
            return hold; //if the token has been reduced to null exit the loop 
        }
        
        //input
        if(strncmp(token, "<", 1)==0){
            hold[0] = "input";
            token = strtok(NULL, deliminators);
            hold[1] = token;
            return hold;
        }

        //output
        if(strncmp(token, ">", 1)==0){
            hold[0] = "output";
            token = strtok(NULL, deliminators);
            hold[1] = token;
            return hold;
        } 

        //pipe
        if(!strncmp(token, "|", 1)){
            hold[0] = "pipe";        
        }
        
        commands++;
        args[commands] = token;
    }
    return hold;
}


//unix shell main code
int UnixLoop(){
    int nonsense = 0;

    char input [BUFSIZ]; //store new input from command line 
    char lastCommand   [BUFSIZ]; // previous command 
    int fdPipe[2];
    //loop until exit() command inputed
    while(true){
        printf("osh> ");  // print to command line 
        fgets(input, BUFSIZ, stdin); //reads input from command line 
        input[strlen(input) - 1] = '\0'; // replace newline with null
       
       //terminate the program when user enters "exit()"
        if(strncmp(input, "exit()", 6) == 0){
            return 0; 
        }
        //check to see if it is a new command 
        if(strncmp(input, "!!", 2)!=0) {
            strcpy(lastCommand, input);
        }
        

        //parent
        pid_t parent = fork();
        char** parsed;
        if(parent != 0){
            if(pause){
                wait(NULL);
            }
        }else{
            char* args[BUFSIZ];    //for command line input 
            memset(args, 0, BUFSIZ * sizeof(char));

            //for user entering "!!" launch most recent command, if no recent command tell user 
            if(strncmp(input, "!!", 2)==0){
                parsed = parser(lastCommand, args);
                if(lastCommand[0]== 0){
                    printf("No commands in history.\n");
                } 
            }else{
                parsed = parser(input,args);
            }
            //check to see if input was specified by the user 
            if(strncmp(parsed[0], "input", 5)==0){
                int fd = open(parsed[1], O_RDONLY);
                read(fd, input,  BUFSIZ * sizeof(char));
                parser(input, args);
            }
            // check to  see if output was specified by the user 
            if(strncmp(parsed[0], "output", 6)==0){
                int fd = open(parsed[1], O_TRUNC | O_CREAT | O_RDWR);
                dup2(fd, STDOUT_FILENO);  
            }

            if(strncmp(parsed[0], "pipe", 1) == 0){//pipe requested by user 
                pid_t pipeID;// ID for the pipe

                //need to change
                int nextArgs = 0; // find the next args in statement
                while(args[nextArgs] != "|"){
                    ++nextArgs;
                }
                // nextArgs++;

                char* leftArgs[BUFSIZ]; //arguments to the left side of the pipe "|"
                char* rightArgs[BUFSIZ]; //arguments to the right side of the pipe "|"
                int index = 0; //index for looping through the input args

                //copy over left side arguments
                while (index<nextArgs){
                    leftArgs[index] = args[index]; //copy the left side arguments
                    index++;
                }
                
                //copy over right side arguments
                index = nextArgs + 1;
                while (args[index] != "\0"){
                    rightArgs[index] = args[index];
                    index++;
                }

                printf("%s args to the left of the pipe", rightArgs);
                printf("%s,args to the right of the pipe", lertArgs); 
                pipeID = fork();

                //parent process writes to the pipe 
                if(pipeID != 0){
                    dup2(fdPipe[1], STDOUT_FILENO); //write 
                    execvp(leftArgs[0], leftArgs);  //call execvp with the leftArgs 
                }

                //child process reads from the pipe 
                if (pipeID == 0){
                    dup2(fdPipe[0], STDIN_FILENO); //read
                    execvp(rightArgs[0], rightArgs); //call execvp with the rightArgs 
                }
            }   

            //call execvp with the function arguments 
            execvp(args[0], args);
        }
    }
return 0;
}

int main(int argc, const char* argv[]){
    UnixLoop();  //loop through the unix shell until termination 
    return 0;
}
