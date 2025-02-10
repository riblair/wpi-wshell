WPI username(s)
jrtinti@wpi.edu 
rpblair@wpi.edu

Part 1
---
Design rationale 
For part 1, I'm going to have a while loop that outputs the prompt with a space and waits for user input. Based on the user input, it will go to a different function that executes the specified call. For each of these functions, there will be error checking for number of arguments as well as the content of the arguments. It will also check if the user typed exit to get out of the shell. It will also be able to handle stdin, files, and pipes. 
Implementation logic 
Go into a while loop printing out what the shortened version of the current directory is. Then it will wait for I/O. There will be multiple if cases looking for what the user typed. If it is cd, we take 2 different versions of what they typed: the full path and the shortened path. The full path is for remembering for if they type pwd later on and the shortened path is for displaying in the prompt. It will change directory based on the input and error in specific situations. For pwd, it displays the path. For echo, it will store the string after echo and then print it out and for exit if they ever type it exits the prompt and the while loop. 
Part 2
---
Design rationale 
At the end of the elifs for part1, it is going to call a helper function called command. This function will take the list of arguments the user entered and then fork and then execve in the child process. It will run the command specified and error out if it does not work properly. 
Implementation logic 
If none of the commands, cd, echo, pwd, or exit are typed, it will call a helper function called command. This will take in the arguments the user called and it will fork. Inside the child process, it will call execve and run the command given. The parent will call waitpid() to wait for the child to finish. 

Part 3
---
Design rationale 
For the first section with history, we are going to keep track of the latest 10 commands run. Then, when the command is inputted from the user, it will display the last 10 commands. 
Implementation logic 
For history, we are going to have an array that stores commands and a count for number of commands run. Every time a command is entered, it will be added to that array. We will take the command and put it in the array at the last index, and then when the next command comes we will shift each command down one. Eventually when we reach 10, the oldest command will be erased. When history is called, it will print out the array, and the start index will depend on the number of total commands run. 
Part 4
---
Design rationale 
First, in the shell we check if the pipe | or redirect operators > or >> are in there. If they are we call their specific function. For the redirects, we write the new command to the file given, and if there is no file we create one. Depending on > or >>, this changes how we write to the file. For the pipe, we open a pipe and make sure to close the sides were not using and pipe between two processes. 
Implementation logic 
For pipe, we check if it is in the input: if it is we call the function. Then, we create a pipe and fork and in the child run the first command, making sure to close off the side we are not using. Then, we fork again, and have the new child run the second command, closing off the correct sides while the parent waits for both of the children. For redirect, we find if there is a redirect and set a Boolean to true if it is >> and false if not. We then call the redirect function. If the Boolean is true, we add the append flag to the file were opening so we append, and if its not we add the o_trunc flag to write over whatever is there. Make sure to close the file as well. 

Part 5
---
Design rationale 
Implementation logic 

We are using the newer version of the tests with 3.out and 6.out not having whitespace 