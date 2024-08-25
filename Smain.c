//Library is being used.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

//Defining global vrabiable
#define PORT 8087
#define SPDF_PORT 8083 
#define STEXT_PORT 8085 
#define SERVER_IP "127.0.0.1"

char *Command = "";

void displayMessage(char *message);
void displayErrorMessage(char *message);
void processCommand(int clientSocketReference);
bool validateusingSTR(char *userInput, char *commandType);
void handleCreateandDownloadTARFiles(char *userCommand, char *fileType, int clientSocketReference);
void sendRequestToCreateTarFilemOtherServer(char *userCommand, char *fileType, int clientSocketReference, int portNumber);
void createTarFile(char *sourceDirectory, char *tarFileLocation, char *fileTypeExtension);
int doesContainProvidedExtension(char *nameOfFile, char *extension);
char *convertPathNameFromSmainToSTextServer(char *directoryPathName, char *contentToReplaceWith);
void createSubDirectoriesAndFiles(char *pathOfDirectory, char *fileExtension);
void createAllFoldersForTheProvidedPath(char *providedlocation);
void createFileAndDirectoryForClient(char *nameOfFile, char *directoryPath);
void handleDisplayFiles(char *userCommand, char *directoryPath, int clientSocketReference);
void callOtherServersToListFiles(char *userCommand, char *directoryPath, int clientSocketReference, int portNumber);
void handleFileRemove(char *userCommand, char *sourceFile, int clientSocketReference);
void removeFileFromMain(char *sourceFile, int clientSocketReference);
void removeFileFromOtherServer(char *userCommand, char *sourceFile, int clientSocketReference, int portNumber);
bool validateCommand(char *userInput, char* commandType);
void handleFileDownload(char *userCommand, char *sourceFile, int clientSocketReference);
void sendRequestToDownloadedFileFromOtherServer(char *userCommand, char *sourceFile, int clientSocketReference, int portNumber);
void sendDownloadedFileFromMainToClient(char *sourceFile, int clientSocketReference);
char *getFileExtension(char *providedFilePath);
void handleFileUpload(char *sourceFile, char *destinationStorage, int clientSocketReference);
char * resolveDirectoryPath( char *directoryPath);
char * resolveSourcenPath( char *fileName);
char * resolveDestinationPath( char *destinationFilePath, char * nameOfFile, int isDestinationADir);
void downloadFileFromServer(char *destinationFilePath, char *sourceFileName, int clientSocketReference);
int createSocketWithPDFandTextServer(int portNumber);
void sendFileToServer( char *destinationStorage,   char *sourceFile, int port);

//Method to display message.
void displayMessage(char *message)
{
    printf(message);
    printf("\n");
}

//Method to display error
void displayErrorMessage(char *message)
{
    perror(message);
}

//Main method from where I am calling to process all the command.
int main() 
{
    int server_fd, newlyCreatedSocket; // new_socket
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        displayErrorMessage("Faild to create the socket.");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        displayErrorMessage("Error happned when setitng the option");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        displayErrorMessage("Error happned during the bind.");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        displayErrorMessage("Error happned while listning.");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    while (1) 
    {
        if ((newlyCreatedSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
        {
            displayErrorMessage("Error happned when accepting the connection.");
            exit(EXIT_FAILURE);
        }

        processCommand(newlyCreatedSocket);
        close(newlyCreatedSocket);
    }

    return 0;
}

// Method where all the command is being executed.
void processCommand(int clientSocketReference)
{
    char spaceToReadData[5024] = {0};
    read(clientSocketReference, spaceToReadData, 5024);
    char destinationStorage[1024];
    char sourceFile[800];
    char userCommand[800];
    // below block is for processing different kind of commands
    if(validateusingSTR(spaceToReadData, "ufile"))
    {
       sscanf(spaceToReadData, "%s %s %s", userCommand, destinationStorage, sourceFile);
       Command = userCommand;
       //calling method to perform the ufile command.
       handleFileUpload(sourceFile, destinationStorage, clientSocketReference);
    }
    else if(validateusingSTR(spaceToReadData, "dfile"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        Command = userCommand;
        //Calling method to handle the dfile command.
        handleFileDownload(userCommand, sourceFile, clientSocketReference);
        
    }
    else if(validateusingSTR(spaceToReadData, "rmfile"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        Command = userCommand;
        //Calling methd to handle rmfile command.
        handleFileRemove(userCommand, sourceFile, clientSocketReference);
        
    }
    else if(validateusingSTR(spaceToReadData, "display"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        Command = userCommand;
        //Calling method to handle display command
        handleDisplayFiles(userCommand, sourceFile, clientSocketReference);
        
    }
    else if(validateusingSTR(spaceToReadData, "dtar"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        Command = userCommand;
        //calling method to handle dtar command.
        handleCreateandDownloadTARFiles(userCommand, sourceFile, clientSocketReference);
        
    }
}

// here using strstr to check if command type exist in the input.
bool validateusingSTR(char *userInput, char *commandType)
{
    return strstr(userInput, commandType) != NULL;
}

// Method to hendle Tar file creation
void handleCreateandDownloadTARFiles(char *userCommand, char *fileType, int clientSocketReference)
{
    char *tarFileName = "cfiles.tar";
    char currentWorkingDir[1024];
    // smain folder from which I  have to get all the c files to create tar
    char sourceDirectory[1024];
    // Folder locaation where tar file is going to be created. means user working directory
    char static tarFileLocation[1024];
    //preparing the varaibles
    if (getcwd(currentWorkingDir, sizeof(currentWorkingDir)) != NULL) 
    {    
        snprintf(sourceDirectory, sizeof(sourceDirectory), "%s/%s", currentWorkingDir, "smain");
        snprintf(tarFileLocation, sizeof(tarFileLocation), "%s/%s", currentWorkingDir, tarFileName);                  
    }

    // If the file type contains .c
    if(validateusingSTR(fileType, ".c"))
    {
        //calling method to create tar file
        createTarFile(sourceDirectory, tarFileLocation, "c");

    }
    else if(validateusingSTR(fileType, ".pdf"))// If the file type contains .pdf
    {
        //sending request to the pdf server by proving dtrar and  fileType.
        snprintf(tarFileLocation, sizeof(tarFileLocation), "%s/%s", currentWorkingDir, "pdf.tar"); 
        sendRequestToCreateTarFilemOtherServer(userCommand, fileType, clientSocketReference, SPDF_PORT);
        displayMessage("Tar file has been created successfully.");

    }
    else if(validateusingSTR(fileType, ".txt"))// If the file type contains .txt
    {
        snprintf(tarFileLocation, sizeof(tarFileLocation), "%s/%s", currentWorkingDir, "text.tar"); 
        //sending request to the pdf server by proving dtrar and  fileType(.txt)
        sendRequestToCreateTarFilemOtherServer(userCommand, fileType, clientSocketReference, STEXT_PORT);
        displayMessage("Tar file has been created successfully.");

    }
   
}

//Sending request to the other server.
void sendRequestToCreateTarFilemOtherServer(char *userCommand, char *fileType, int clientSocketReference, int portNumber)
{
    // Creating the socket to send the request.
   int socketReference = createSocketWithPDFandTextServer(portNumber);
    char commandForServer[256];
    //preparing the message which needs to be send to the pdf or Text server.
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, fileType);
    // Sending the command.
    send(socketReference, commandForServer, strlen(commandForServer), 0);
}

//Careating the tar file.
void createTarFile(char *sourceDirectory, char *tarFileLocation, char *fileTypeExtension)
{
    char createTarCommand[1024];
    // Commad to get all the files in sourceDirectory with the extension .c and then create the tar file in user working directory.
    snprintf(createTarCommand, sizeof(createTarCommand), "find %s -type f -name \"*.%s\" | tar -cvf %s -T -", sourceDirectory, fileTypeExtension, tarFileLocation);

    FILE *referenceOfFile = popen(createTarCommand, "r");
    char spaceToReadData[256];
    while (fgets(spaceToReadData, sizeof(spaceToReadData), referenceOfFile) != NULL)
    {
        printf("%s", spaceToReadData);
    }

// In case of error.
    if (pclose(referenceOfFile) == -1)
    {
        displayErrorMessage("Error while closing the file");
    }
}

//Checking the extension
int doesContainProvidedExtension(char *nameOfFile, char *extension)
{
    char *positionOfDotExtension = strrchr(nameOfFile, '.');
    return (positionOfDotExtension && strcmp(positionOfDotExtension + 1, extension) == 0);
}

// Preparing the path
char *convertPathNameFromSmainToSTextServer(char *directoryPathName, char *contentToReplaceWith)
{
    // Calculating the length of the content which I have to replace. eg. length of smain
    int contentToReplaceWithLength = strlen(contentToReplaceWith);
    char *updatedDirectoryPathName = malloc(strlen(directoryPathName) + 1);

    char *position = directoryPathName;
    char *reservePointer = updatedDirectoryPathName;

    while ((position = strstr(position, contentToReplaceWith)) != NULL)
    {
        strncpy(reservePointer, directoryPathName, position - directoryPathName);
        reservePointer += position - directoryPathName;

        position += contentToReplaceWithLength;
        directoryPathName = position;
    }
    strcpy(reservePointer, directoryPathName);

    // Eg. /home/Kunwar2/ServerProject/smain/Folder1/Folder2 is being converted to the below string
    // /home/Kunwar2/ServerProject/Folder1/Folder2
    return updatedDirectoryPathName;
}

//Method to iterate over all the subdirectories in pathOfDirectory and then calling createFileAndDirectoryForClient 
// to create the folder and files.
void createSubDirectoriesAndFiles(char *pathOfDirectory, char *fileExtension)
{
    struct dirent *entry;
    struct stat path_stat;
    char folderLocation[1024];
    DIR *directoryReference = opendir(pathOfDirectory);

    while ((entry = readdir(directoryReference)))
    {
        // If its a hidden file no need to check
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        // Preaparing the folderlocatation here pathOfDirectory is /home/Kunwar2/ServerProject and entry->d_name smain
        // then the folderlocatation = /home/Kunwar2/ServerProject/smain
        snprintf(folderLocation, sizeof(folderLocation), "%s/%s", pathOfDirectory, entry->d_name);

        if (stat(folderLocation, &path_stat) == -1)
        {
            displayErrorMessage("Error in the file stat");
            continue;
        }
        // Checking if its a directory
        if (S_ISDIR(path_stat.st_mode))
        {
            // Calling recursively to go to inside the folder eg from  /home/Kunwar2/ServerProject to 
            // /home/Kunwar2/ServerProject/smain
            createSubDirectoriesAndFiles(folderLocation, fileExtension);
        }
        else if (S_ISREG(path_stat.st_mode)) // if its a regular file
        {
            if (doesContainProvidedExtension(entry->d_name, fileExtension)) // if the file has the provided extension
            {
                // retriving the directory name 
                char *directoryPathName = dirname(folderLocation);
                char *sourceDirectoryPathName = "";
                // if the directory name contains "smain/" calling convertPathNameFromSmainToSTextServer to remove "smain/ from the  directory path name
                if (strstr(directoryPathName, "smain/") != NULL) 
                    sourceDirectoryPathName = convertPathNameFromSmainToSTextServer(directoryPathName, "smain/");
                else
                    sourceDirectoryPathName = convertPathNameFromSmainToSTextServer(directoryPathName, "smain");
                // now passing the updated path name to create the directory recussilvely.
                createFileAndDirectoryForClient(entry->d_name, sourceDirectoryPathName);
            }
        }
    }

    closedir(directoryReference);
}

//Mehtod to create all folder and files
void createAllFoldersForTheProvidedPath(char *providedlocation)
{
    char prepareFolderPath[800];
    char backupOfPrepareFolderPath[800];
    char *token;
    // Making a duplicate copy of providedlocation
    //   /home/kunwar2/ServerProject
    char *backupOfProvidedlocation = strdup(providedlocation);
    //prepareFolderPath has  / 
    snprintf(prepareFolderPath, sizeof(prepareFolderPath), "%s", "/");
    // backupOfPrepareFolderPath has /
    snprintf(backupOfPrepareFolderPath, sizeof(backupOfPrepareFolderPath), "%s", prepareFolderPath);
    token = strtok(backupOfProvidedlocation, "/");
    while (token != NULL)
    { 
        // first iteration /home
        //second iteration /home/kunwar2
        snprintf(prepareFolderPath, sizeof(prepareFolderPath), "%s%s/", backupOfPrepareFolderPath, token);
        // in each iteration will create folder
        mkdir(prepareFolderPath, 0755);
        snprintf(backupOfPrepareFolderPath, sizeof(backupOfPrepareFolderPath), "%s", prepareFolderPath);
        token = strtok(NULL, "/");
    }
}

//Create folder and files for the client
void createFileAndDirectoryForClient(char *nameOfFile, char *directoryPath)
{
    // calling the method to create all the path recursively.
    createAllFoldersForTheProvidedPath(directoryPath);
    char completePathOfFile[256];
    // Preparing the complete path of the file.
    snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", directoryPath, nameOfFile);
    // when open in write mode, file will be automatically created.
    FILE *file = fopen(completePathOfFile, "w");
}

// Handling the command to display files.
void handleDisplayFiles(char *userCommand, char *directoryPath, int clientSocketReference)
{
    // User has provided the directoryPath, I am calling the below method to remove ~ from the whole path
   char *completeDirectoryPath = resolveDirectoryPath(directoryPath);
   createSubDirectoriesAndFiles(completeDirectoryPath, "c");
   callOtherServersToListFiles(userCommand, directoryPath, clientSocketReference, SPDF_PORT);
   callOtherServersToListFiles(userCommand, directoryPath, clientSocketReference, STEXT_PORT);

   
}
//Calling the other server to list all the files
void callOtherServersToListFiles(char *userCommand, char *directoryPath, int clientSocketReference, int portNumber)
{
    // Creating the socket
   int socketReference = createSocketWithPDFandTextServer(portNumber);
    char commandForServer[256];
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, directoryPath);
    // sending the request to the provided port number 
    send(socketReference, commandForServer, strlen(commandForServer), 0);

}

// Method to handle the rmfile command
void handleFileRemove(char *userCommand, char *sourceFile, int clientSocketReference)
{
    char * fileExtension = getFileExtension(sourceFile);
    if(validateCommand(fileExtension, "c"))
    {
        removeFileFromMain(sourceFile, clientSocketReference);
    }
    else if(validateCommand(fileExtension, "pdf"))
    {
       removeFileFromOtherServer(userCommand, sourceFile, clientSocketReference, SPDF_PORT);

    }
    else if(validateCommand(fileExtension, "txt"))
    {
        removeFileFromOtherServer(userCommand, sourceFile, clientSocketReference, STEXT_PORT);
    }
   
}

// Method to delete a file for the main server.
void removeFileFromMain(char *sourceFile, int clientSocketReference)
{
    // extracting the file name from the sourceFile
    char *nameOfFile = basename(sourceFile);
    // calling resolveDestinationPath to get the complete path of the file which I am going to delete.
    char *fileToDelete = resolveDestinationPath(sourceFile, nameOfFile, 0);
    // calling remove to delte it.
    if (remove(fileToDelete) == 0)
    {
        displayMessage("Requested file has been deleted sucessfully.");
    }
    else
    {
        displayErrorMessage("Error happned while deleting the file");
    }
}

// Sending request to other server to remove the file.
void removeFileFromOtherServer(char *userCommand, char *sourceFile, int clientSocketReference, int portNumber)
{
   int socketReference = createSocketWithPDFandTextServer(portNumber);
    char commandForServer[256];
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, sourceFile);
    send(socketReference, commandForServer, strlen(commandForServer), 0);
}

// Method to validate the command type means checking if it exist in the the user input
bool validateCommand(char *userInput, char* commandType)
{
   return strcmp(userInput, commandType) == 0;
}

//Method to handle File download.
void handleFileDownload(char *userCommand, char *sourceFile, int clientSocketReference)
{
    // Calling the method to get the extension of the file.
    char * fileExtension = getFileExtension(sourceFile);
    if(validateCommand(fileExtension, "c"))
    {
        sendDownloadedFileFromMainToClient(sourceFile, clientSocketReference);
    }
    else if(validateCommand(fileExtension, "pdf"))
    {
        sendRequestToDownloadedFileFromOtherServer(userCommand, sourceFile, clientSocketReference, SPDF_PORT);

    }
    else if(validateCommand(fileExtension, "txt"))
    {
        sendRequestToDownloadedFileFromOtherServer(userCommand, sourceFile, clientSocketReference, STEXT_PORT);

    }
   
}

// Sending request to the other server to get the download file and then sending it to the client
void sendRequestToDownloadedFileFromOtherServer(char *userCommand, char *sourceFile, int clientSocketReference, int portNumber)
{
    char spaceToReadData[1024];
    size_t noOfBytesRead;

    // Creating the sockt to communicate with pdf or text server.
   int socketReference = createSocketWithPDFandTextServer(portNumber);
    char commandForServer[256];
    snprintf(commandForServer, sizeof(commandForServer), "%s %s", userCommand, sourceFile);
    // Sending commad to the text or pdf server
    send(socketReference, commandForServer, strlen(commandForServer), 0);
    // Reading the text that is being send by the pdf or text server
    while ((noOfBytesRead = read(socketReference, spaceToReadData, sizeof(spaceToReadData))) > 0) 
    {
        char commandForClient[256];
        snprintf(commandForClient, sizeof(commandForClient), "%s", spaceToReadData);
        //Snding the text to the client which is stored in the buffer.
        send(clientSocketReference, commandForClient, strlen(commandForClient), 0);
    }
    displayMessage("Data has been sent for the requested file.");

}

// This method is purly for the smain server to send the requested file to the client which he/she has requested to download form smain
void sendDownloadedFileFromMainToClient(char *sourceFile, int clientSocketReference)
{
    //Extracting the file name
    char *nameOfFile = basename(sourceFile);
    // preparing the path from where smain server has to read the file 
    char *fileToRead = resolveDestinationPath(sourceFile, nameOfFile, 0);
    int noOfBytesRead;
    char spaceToReadData[1024]; //A buffer where the data read from the file gets stored.
    // Opening the file in the read mode.
    FILE *referenceOfFile = fopen(fileToRead, "rb");
    while ((noOfBytesRead = fread(spaceToReadData, 1, sizeof(spaceToReadData), referenceOfFile)) > 0)
    {
        // reading it and sending it to client.
        send(clientSocketReference, spaceToReadData, noOfBytesRead, 0);
    }
    displayMessage("Requested file content has been sent sucessfully.");
    fclose(referenceOfFile);
}

// Method to get the file extension
char *getFileExtension(char *providedFilePath) 
{
     char *locationOfExtension = strrchr(providedFilePath, '.'); 
    return locationOfExtension + 1; 
}

// Method to handle file upload.
void handleFileUpload(char *sourceFile, char *destinationStorage, int clientSocketReference) 
{
    // getting the file extension
    char * fileExtension = getFileExtension(sourceFile);
    if(validateCommand(fileExtension, "c"))
    {
        downloadFileFromServer(destinationStorage, sourceFile, clientSocketReference);
    }
    else if(validateCommand(fileExtension, "pdf"))
    {
        sendFileToServer(destinationStorage, sourceFile, SPDF_PORT);

    }
    else if(validateCommand(fileExtension, "txt"))
    {
        sendFileToServer(destinationStorage, sourceFile, STEXT_PORT);

    }
}

//Method to resolve the directory path by removing the ~
char * resolveDirectoryPath( char *directoryPath)
{
   char userWorkingDirectory[1024];
   char static directoryOfFile[1024];
   char filePathAfterRemovingTilde[256];
   if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL) 
   {
        char *start = strchr(directoryPath, '~'); 
        strncpy(filePathAfterRemovingTilde, start+1, sizeof(filePathAfterRemovingTilde) - 1);  
        snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s", userWorkingDirectory, filePathAfterRemovingTilde);
    }
    return directoryOfFile;
}

// Method to resolve the destination path
char * resolveDestinationPath( char *destinationFilePath, char * nameOfFile, int isDestinationADir)
{
   char userWorkingDirectory[1024];
   char directoryOfFile[1024];
   char static completePathOfFile[1024];
   char filePathAfterRemovingTilde[256];
   // getting the user working directory
   if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL) 
   {
        char *start = strchr(destinationFilePath, '~'); 
        strncpy(filePathAfterRemovingTilde, start+1, sizeof(filePathAfterRemovingTilde) - 1);  
        snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s", userWorkingDirectory, filePathAfterRemovingTilde);
        // completePathOfFile = directory path + file name.
        snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", directoryOfFile, nameOfFile);
       
    }
    return isDestinationADir ? completePathOfFile : directoryOfFile;
}

// Method to reolve the source path
char * resolveSourcenPath( char *fileName)
{
   char userWorkingDirectory[1024];
   char static completePathOfFile[1024];
   if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL) 
   {
        snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", userWorkingDirectory, fileName);
   }
    return completePathOfFile;
}

// Method to download the file form the server.
void downloadFileFromServer(char *destinationFilePath, char *sourceFileName, int clientSocketReference)
{

    char userWorkingDirectory[1024];
    char directoryOfFile[1024];
    char *nameOfFile = basename(sourceFileName);
    char *completePathOfFile = resolveDestinationPath(destinationFilePath, nameOfFile, 1);
    char *directoryPathName = dirname(completePathOfFile);
    // Creating the file and the all the needed directory 
    createFileAndDirectoryForClient(nameOfFile, directoryPathName);

    snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s", directoryPathName, nameOfFile);

    FILE *referenceOfFile;
    // Opening the file in write mode.
    referenceOfFile = fopen(directoryOfFile, "wb");
    char spaceToReadData[1024];
    int noOfBytesRead;
    // Reading from the socket
    while ((noOfBytesRead = read(clientSocketReference, spaceToReadData, sizeof(spaceToReadData))) > 0)
    {
        // Writing to the file.
        fwrite(spaceToReadData, 1, noOfBytesRead, referenceOfFile);
    }

    fclose(referenceOfFile);
    displayMessage("File has been saved on the server.");
}

int createSocketWithPDFandTextServer(int portNumber)
{
    int socketReference;
    struct sockaddr_in server_addr;
    if ((socketReference = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        displayErrorMessage("Error happned while creating the socket.");
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNumber);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    connect(socketReference, (struct sockaddr *)&server_addr, sizeof(server_addr));
    return socketReference;
}
// Sending the file to the server.
void sendFileToServer( char *destinationStorage,   char *sourceFile, int port) 
{
    int socketReference;
    struct sockaddr_in server_addr;
    char *fileToRead = resolveSourcenPath(sourceFile);
    FILE *referenceOfFile = fopen(fileToRead, "rb");
    //Creating the socket to communicate with pDF and Text server
    socketReference = createSocketWithPDFandTextServer(port);
   
    char commandForServer[256];
    snprintf(commandForServer, sizeof(commandForServer), "%s %s %s", destinationStorage, sourceFile, Command);
    // Sending the command to the pdf/ text server.
    send(socketReference, commandForServer, strlen(commandForServer), 0);

    char spaceToReadData[1024];
    int noOfBytesRead;
    // Reading the file and sending it to pdf/text server.
    while ((noOfBytesRead = fread(spaceToReadData, 1, sizeof(spaceToReadData), referenceOfFile)) > 0) 
    {
        send(socketReference, spaceToReadData, noOfBytesRead, 0);
    }

    fclose(referenceOfFile);
    close(socketReference);
    printf("File sent to server on port %d: %s\n", port, destinationStorage);
}
