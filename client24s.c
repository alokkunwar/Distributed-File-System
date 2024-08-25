#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <stdbool.h>

#define SERVER_IP "127.0.0.1"  
#define SERVER_PORT 8087     
#define MAX_LENGTH_ARGUMENT 6
char *listOfArguments[MAX_LENGTH_ARGUMENT];  
int ParametersCount = 0;

// All the method declaration
void tarnsferFileToMainServer(  char * userCommand,  char *destinationFilePath,  char *fileName);
void downloadFileFromServer( char *userCommand,  char *fileName);
bool noOfParameters(char *providedInput, char *delimType);
void closeSocket(int socketReference);
void displayMessage(char *message);
void displayErrorMessage(char * message);
char * resolveSourcenPath( char *fileName);
void prepareMessageToDisplay4Parm(char *parameter1, char *parameter2);
bool validateCommand(char *userInput, char* commandType);
void processCommand(char *userInput[]);
void downloadTARFileFromServer( char *userCommand,  char *fileType);
void displayFilesFromServer( char *userCommand,  char *pathName);
void removeFileFromServer( char *userCommand,  char *fileName);
int createSocket();

// Method to display the message
void displayMessage(char *message)
{
    printf(message);
    printf("\n");
}
//Method to display the error
void displayErrorMessage(char * message)
{
     perror(message);
}

//Method to check the number of parameters
bool noOfParameters(char *providedInput, char *delimType)
{
    int parmCount = 0;
    char * savePtr = NULL;
    ParametersCount = 0;
    //creating the tokens
    char *listOfToken = strtok_r(providedInput, delimType, &savePtr);
    while(listOfToken != NULL) 
    {
      listOfArguments[parmCount++] = listOfToken;
      listOfToken = strtok_r(NULL, delimType, &savePtr);
    } 
    listOfArguments[parmCount] = NULL;
    ParametersCount = parmCount;
    return parmCount >=2 && parmCount <=3;
}

// Main method
int main(int argc, char *argv[]) {

    
    char providedUserCommand[500]; 
    char *argumentForSpecialChar[500];
    // While loop to take the user command continiously
    while(1)
    {
        printf("client24s$ ");
        //Fulsing out the buffer
        fflush(stdout); 
        // Taking the user input
        if (fgets(providedUserCommand, sizeof(providedUserCommand), stdin) == NULL)
        {
            displayMessage("Error happened while reading the user command."); 
            continue;
        }

        //Replacing the \n with end line. 
        providedUserCommand[strcspn(providedUserCommand, "\n")] = '\0';
        char tempProvidedUserCommand[500];
    
        strcpy(tempProvidedUserCommand, providedUserCommand); 
        // Validating the no of parameter and if not having valid parameter showing the below error message.
         if(!noOfParameters(tempProvidedUserCommand, " "))
        {
            displayMessage("Number of parameter should be between 2 and 3. Please provide correct input.");
        }
        else
        {
            // If command is right, calling the method to process it.
          processCommand(listOfArguments);
        }
    }
    return EXIT_SUCCESS;
}

// Resolving the source path
char * resolveSourcenPath( char *fileName)
{
   char userWorkingDirectory[1024];
   char static completePathOfFile[1024];
   if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL) 
   {
        // Complete path =  user working directory + file Name
        snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", userWorkingDirectory, fileName);
    }
    return completePathOfFile;
}

// Prepare the  message  to display where i have to resolve the 4 params
void prepareMessageToDisplay4Parm(char *parameter1, char *parameter2)
{
        char UserMessage[1024];
        // Arranging the general message to display to the client.
        snprintf(UserMessage, sizeof(UserMessage), "%s %s %s %s %s", "Forrwarding parametrer ",parameter1, "and", parameter2, "to main server.");
        displayMessage(UserMessage);
}

// Method to check the command type
bool validateCommand(char *userInput, char* commandType)
{
   return strcmp(userInput, commandType) == 0;
}

// Calloing the method to process the command
void processCommand(char *userInput[])
{
    // Below block is to process different types of the command
    if(validateCommand(userInput[0], "ufile"))
    {
         char *userCommand = userInput[0];
         char *fileName = userInput[1];
         char *destinationFilePath = userInput[2];
        tarnsferFileToMainServer(userCommand, destinationFilePath, fileName);
    }
    else if(validateCommand(userInput[0], "dfile"))
    {
         char *userCommand = userInput[0];
         char *fileName = userInput[1];
        downloadFileFromServer(userCommand, fileName);
    }
    else if(validateCommand(userInput[0], "rmfile")) 
    {
         char *userCommand = userInput[0];
         char *fileName = userInput[1];
        removeFileFromServer(userCommand, fileName);
    }
    else if(validateCommand(userInput[0], "display")) 
    {
         char *userCommand = userInput[0];
         char *pathName = userInput[1];
        displayFilesFromServer(userCommand, pathName);
    }
    else if(validateCommand(userInput[0], "dtar")) 
    {
         char *userCommand = userInput[0];
         char *fileType = userInput[1];
        downloadTARFileFromServer(userCommand, fileType);
    }
    else
    {
        // Displaying the error message.
        displayMessage("Command is not valid. Please enter a valid command.");

    }
}
// Method to handle dtar command.
void downloadTARFileFromServer( char *userCommand,  char *fileType)
{
    // For the below three extension user can request to create the tar file from the server.
    if(strstr(fileType,".c") != NULL || strstr(fileType,".pdf") != NULL || strstr(fileType,".txt") != NULL  )
    {
        int socketReference = createSocket();
        char commandForServer[1024];
        prepareMessageToDisplay4Parm(userCommand, fileType);
        // Preparing the message which needs to be send to the smain server
        snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, fileType);
        // Sending the message.
        send(socketReference, commandForServer, strlen(commandForServer), 0);
        displayMessage("Tar file has been downloaded.");
        closeSocket(socketReference);
    }
    else
        displayMessage("File type is not valied. Please enter a valid file type.");
}
// Method to handle the display command
void displayFilesFromServer( char *userCommand,  char *pathName)
{

    int socketReference = createSocket();
    char commandForServer[1024];
    prepareMessageToDisplay4Parm(userCommand, pathName);
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, pathName);
    send(socketReference, commandForServer, strlen(commandForServer), 0);
    displayMessage("Display operation is sucessful.");
    closeSocket(socketReference);

}
// Method to send the requst to the server to handle the rmfile command
void removeFileFromServer( char *userCommand,  char *fileName)
{

    int socketReference = createSocket();
    char commandForServer[1024];
    prepareMessageToDisplay4Parm(userCommand, fileName);
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, fileName);
    send(socketReference, commandForServer, strlen(commandForServer), 0);
    displayMessage("File has been sucessfully deleted.");
    closeSocket(socketReference);

}
// Mthod to downlod the files.
void downloadFileFromServer( char *userCommand,  char *fileName)
{
    FILE *referenceOfFile;
    char spaceToReadData[1024];
    size_t noOfBytesRead;

    int socketReference = createSocket();
    char commandForServer[1024];
    prepareMessageToDisplay4Parm(userCommand, fileName);

    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, fileName);
    send(socketReference, commandForServer, strlen(commandForServer), 0);

    char *fileNameOnClientSide = basename(fileName);
     if (access(fileNameOnClientSide, F_OK) == -1) 
     {
      referenceOfFile = fopen(fileNameOnClientSide, "w");  
        if (referenceOfFile == NULL) 
        {
            displayErrorMessage("Error happned while creating the file.");
        }
    }
    else
    {
      referenceOfFile = fopen(fileNameOnClientSide, "wb");
      if (referenceOfFile == NULL) 
      {
        displayErrorMessage("Error happned while opening the file.");
        return;
      }
    }

    while ((noOfBytesRead = read(socketReference, spaceToReadData, sizeof(spaceToReadData))) > 0) {
        fwrite(spaceToReadData, 1, noOfBytesRead, referenceOfFile);
    }

    fclose(referenceOfFile);
    displayMessage("File has been downloaded.");
    closeSocket(socketReference);

}
// Method to create the socket.
int createSocket()
{
    int socketReference;
    struct sockaddr_in server_addr;

    if ((socketReference = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        displayErrorMessage("Error happned while creating the socket.");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        displayErrorMessage("It seems address is not valid.");
        close(socketReference);
        exit(EXIT_FAILURE);
    }

    if (connect(socketReference, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        displayErrorMessage("Error happned while connecting to server");
        close(socketReference);
        exit(EXIT_FAILURE);
    }

    return socketReference;

}

// Method to close the socket
void closeSocket(int socketReference)
{
    close(socketReference);
}

// Method to handle the ufile command
void tarnsferFileToMainServer(  char *userCommand,  char *destinationFilePath,  char *fileName) 
{
    FILE *referenceOfFile;
    char spaceToReadData[1024];
    size_t noOfBytesRead;

    int socketReference = createSocket();

    char *fileToRead = resolveSourcenPath(fileName);
    if ((referenceOfFile = fopen(fileToRead, "rb")) == NULL) 
    {
        displayErrorMessage("Error happned while opening the file.");
        closeSocket(socketReference);
        exit(EXIT_FAILURE);
    }

    char commandForServer[1024];
    printf("Forrwarding %s %s and %s\n", userCommand, destinationFilePath, fileName);
    snprintf(commandForServer, sizeof(commandForServer), "%s %s %s", userCommand, destinationFilePath, fileName);
    send(socketReference, commandForServer, strlen(commandForServer), 0);

    while ((noOfBytesRead = fread(spaceToReadData, 1, sizeof(spaceToReadData), referenceOfFile)) > 0)
    {
        send(socketReference, spaceToReadData, noOfBytesRead, 0);
    }

    fclose(referenceOfFile);
    closeSocket(socketReference);
    displayMessage("File uploaded successfully.");
}
