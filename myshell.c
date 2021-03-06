/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

extern char **getaline();

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal) {
  int status;
  int result = wait(&status);

  printf("Wait returned %d\n", result);
}



/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++) ;

  if(args[i-1][0] == '&' && args[i][0] != '&') {
    free(args[i-1]);
    args[i-1] = NULL;
    return 1;
  } else {  
    return 0;
  }
  
  return 0;
}


int and_command(char **args, char **command){
  int i;
  int j;
  int k = 0;
  
  for(i = 1; args[i] != NULL; i++){
    if(args[i-1][0] == '&' && args[i][0] == '&'){
    
      free(args[i-1]);
      args[i-1] = NULL;
      free(args[i]);
      args[i] = NULL;
      for(j = i+1; args[j] != NULL;j++){
        command[k] = args[j];
        k++;
      }
      return 1;
    }
  }
  return 0;
}

int or_command(char **args){
  int i;
  
  for(i = 1; args[i] != NULL; i++){
    if(args[i-1][0] == '|' && args[i][0] == '|'){
      free(args[i-1]);
      args[i-1] = NULL;
      free(args[i]);
      args[i] = NULL;
      return 1;
    }
  }
  return 0;
}


/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args) {
  if(strcmp(args[0], "exit") == 0) {
    exit(0);
  }

  return 0;
}

/* 
 * Do the command
 */
int do_command(char **args, char **command, int block,
          int andVal, int orVal,
	       int input, char *input_filename,
	       int output, char *output_filename, 
         int append, char *append_filename) {
  
  int result;
  pid_t child_id;
  int status;

  // Fork the child process
  child_id = fork();

  // Check for errors in fork()
  switch(child_id) {
  case EAGAIN:
    perror("Error EAGAIN: ");
    return;
  case ENOMEM:
    perror("Error ENOMEM: ");
    return;
  }

  if(child_id == 0) {

    // Set up redirection in the child process
     
    
    if(input)
      freopen(input_filename, "r", stdin);

    if(output)
      freopen(output_filename, "w+", stdout);
      
      if(append)
      freopen(append_filename, "a", stdout);

    // Execute the command
    result = execvp(args[0], args);
    printf("%d\n",result); 
    /*
    if(andVal){
      int pid;
       pid = fork();
        if(pid==-1) {
           perror("Failed to fork");
           return 1;
        }
        else if(pid ==0){
           int i;
      
            printf("here");
            
            for(i = 0; command[i] != NULL; i++){
                args[i] = command[i];
                free(command[i]);
            }
            result = execvp(args[0], args);
       }
    }  
    else{
      result = execvp(args[0], args);
    }
    */
    
    exit(-1);
  }
  
  // Wait for the child process to complete, if necessary
  if(block) {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0);
  }
}
/** 
 * This is the function for append 
 */ 
int append_file(char **args, char **append_filename) { 
  int i; 
  int j; 
   
  for(i = 0; args[i] != NULL; i++) { 
   
    // Look for the >> 
    
    if(args[i][0] == '>' && args[i+1][0] == '>') { 
      
 
      // Get the filename  
      //printf("%s", args[4]); 
      if(args[i+2] != NULL) { 
	      *append_filename = args[i+2]; 
      } else { 
	      return -1; 
      } 
      // Adjust the rest of the arguments in the array 
      for(j = i; args[j-1] != NULL; j++) { 
	      args[j] = args[j+2]; 
       //printf("    %s    ", args[j]);
      } 
      return 1; 
    } 
  } 
  return 0; 
} 

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the <
    if(args[i][0] == '<') {
      free(args[i]);

      // Read the filename
      if(args[i+1] != NULL) {
	*input_filename = args[i+1];
      } else {
	return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the >
    if(args[i][0] == '>') {
      free(args[i]);

      // Get the filename 
      if(args[i+1] != NULL) {
	        *output_filename = args[i+1];
      } else {
	        return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	        args[j] = args[j+2];

      }

      return 1;
    }
  }

  return 0;
}
/*
 * The main shell function
 */ 
main() {
  int i;
  char **args; 
  char **command;
  int result;
  int block;
  int andVal;
  int orVal;
  int output;
  int input;
  int append;
  char *output_filename;
  char *input_filename;
  char *append_filename; 

  // Set up the signal handler
  sigset(SIGCHLD, sig_handler);

  // Loop forever
  while(1) {

    // Print out the prompt and get the input
    printf("->");
    args = getaline();

    // No input, continue
    if(args[0] == NULL)
      continue;

    // Check for internal shell commands, such as exit
    if(internal_command(args))
      continue;

    // Check for an ampersand
    block = (ampersand(args) == 0);

    //check for double ampersand
    command = malloc(30 * sizeof(char*));
    andVal = and_command(args, command);
    
    //check for double pipe
    orVal = or_command(args);
    


    // Check for redirected input
    input = redirect_input(args, &input_filename);

    switch(input) {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting input from: %s\n", input_filename);
      break;
    }
    //check for appending 
   append = append_file(args, &append_filename); 
 
    switch(append) { 
    case -1: 
      printf("Syntax error!\n"); 
      continue; 
      break; 
    case 0: 
      break; 
    case 1: 
      printf("Appending output to: %s\n", append_filename); 
      break; 
    } 
    // Check for redirected output
    output = redirect_output(args, &output_filename);

    switch(output) {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting output to: %s\n", output_filename);
      break;
    }

    // Do the command
    
      
    do_command(args, command, block, 
          andVal, orVal,
	       input, input_filename, 
	       output, output_filename,
         append, append_filename);
  /*       
    int i;
    if(andVal == 1){
      printf("here0");
      for(i = 0; command[i] != NULL; i++){
          args[i] = command[i];
          free(command[i]);
          printf("here1");
      }
        do_command(args, command, block, 
          andVal, orVal,
	       input, input_filename, 
	       output, output_filename,
         append, append_filename);
      }
    */
    
  }
}

