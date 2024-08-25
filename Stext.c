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

#define PORT 8085
#define SERVER_IP "127.0.0.1"
// The implementation of Stext and SPdf are same. I have provided the comment in the Spdf file. Please refer it for more details.
// Method declaration.
char *resolveDirectoryPath(char *destinationFilePath, int isDestinationDir);
void displayMessage(char *message);
void displayErrorMessage(char *message);
char *fixFilePath(char *nameOfFile);
char *resolveDestinationPath(char *destinationFilePath, char *nameOfFile, int isDestinationDir);
char *resolveSourcenPath(char *nameOfFile);
bool validateCommand(char *userInput, char *commandType);
void processCommand(int clientSocketReference);
void handleCreateandDownloadTARFiles(char *userCommand, char *fileType, int clientSocketReference);
void createTarFile(char *sourceDirectory, char *tarFileLocation, char *fileTypeExtension);
int doesContainProvidedExtension(char *nameOfFile, char *extension);
char *convertPathNameFromSmainToSTextServer(char *directoryPathName, char *contentToReplaceWith);
void createSubDirectoriesAndFiles(char *pathOfDirectory, char *fileExtension);
void createAllFoldersForTheProvidedPath(char *providedlocation);
void createFileAndDirectoryForClient(char *nameOfFile, char *directoryPath);
void handleDisplayFiles(char *userCommand, char *directoryPath, int clientSocketReference);
char *resolveDirectoryPath(char *destinationFilePath, int isDestinationDir);
void removeFileFromPDFServer(char *sourceFile, int clientSocketReference);
int createSocketWithPDFandTextServer(int portNumber);
void sendDownloadedFileFromPDFToClient(char *sourceFile, int clientSocketReference);
void downloadFileFromServer(char *destinationFilePath, char *sourceFileName, int clientSocketReference);
void handleFileUpload(int clientSocketReference);

// Method to display the message.
void displayMessage(char *message)
{
    printf(message);
    printf("\n");
}

// Method to display the error
void displayErrorMessage(char *message)
{
    perror(message);
}

// Method to fix the file path
char *fixFilePath(char *nameOfFile)
{
    char static endResult[256];
    char *StartPointer = strchr(nameOfFile, '.');
    int extensionPosition = StartPointer - nameOfFile;
    strncpy(endResult, nameOfFile, (extensionPosition + 4));
    endResult[extensionPosition + 4] = '\0';
    return endResult;
}

// Method to  resolve the destination path to store the files.
char *resolveDestinationPath(char *destinationFilePath, char *nameOfFile, int isDestinationDir)
{
    char userWorkingDirectory[1024];
    char directoryOfFile[1024];
    char static completePathOfFile[1024];
    char filePathAfterRemovingSmain[256];
    if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL)
    {
        int length = strlen("~smain");
        char *start = strchr(destinationFilePath, '~');

        strncpy(filePathAfterRemovingSmain, start + length, sizeof(filePathAfterRemovingSmain) - 1);

        if (filePathAfterRemovingSmain != '\0')
        {
            snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s%s/", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
            if (!isDestinationDir)
                snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s%s", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
        }
        else
        {
            snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s/%s", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
        }
        snprintf(completePathOfFile, sizeof(completePathOfFile), "%s%s", directoryOfFile, nameOfFile);
    }
    return isDestinationDir ? completePathOfFile : directoryOfFile;
}

// Method to resolve the source path
char *resolveSourcenPath(char *nameOfFile)
{
    char userWorkingDirectory[1024];
    char static completePathOfFile[1024];
    if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL)
    {
        snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", userWorkingDirectory, nameOfFile);
    }
    return completePathOfFile;
}

// Method to validate the command
bool validateCommand(char *userInput, char *commandType)
{
    return strstr(userInput, commandType) != NULL;
}

// Method to process the command
void processCommand(int clientSocketReference)
{
    char spaceToReadData[5024] = {0};
    read(clientSocketReference, spaceToReadData, 5024);
    char destinationStorage[1024];
    char sourceFile[800];
    char userCommand[800];
    // Below block is to process different kind of command
    if (validateCommand(spaceToReadData, "ufile"))
    {
        sscanf(spaceToReadData, "%s %s %s", destinationStorage, sourceFile, userCommand);
        char *inputFile = fixFilePath(sourceFile);
        downloadFileFromServer(destinationStorage, inputFile, clientSocketReference);
    }
    else if (validateCommand(spaceToReadData, "dfile"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        sendDownloadedFileFromPDFToClient(sourceFile, clientSocketReference);
    }
    else if (validateCommand(spaceToReadData, "rmfile"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        removeFileFromPDFServer(sourceFile, clientSocketReference);
    }
    else if (validateCommand(spaceToReadData, "display"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        handleDisplayFiles(userCommand, sourceFile, clientSocketReference);
    }
    else if (validateCommand(spaceToReadData, "dtar"))
    {
        sscanf(spaceToReadData, "%s %s", userCommand, sourceFile);
        handleCreateandDownloadTARFiles(userCommand, sourceFile, clientSocketReference);
    }
}

// Method to create and download the tar file
void handleCreateandDownloadTARFiles(char *userCommand, char *fileType, int clientSocketReference)
{

    char *tarFileName = "text.tar";
    char currentWorkingDir[1024];
    char sourceDirectory[1024];
    char static tarFileLocation[1024];
    if (getcwd(currentWorkingDir, sizeof(currentWorkingDir)) != NULL)
    {
        snprintf(sourceDirectory, sizeof(sourceDirectory), "%s/%s", currentWorkingDir, "stext");
        snprintf(tarFileLocation, sizeof(tarFileLocation), "%s/%s", currentWorkingDir, tarFileName);
    }
    createTarFile(sourceDirectory, tarFileLocation, "txt");
}

// Method to create the tar file
void createTarFile(char *sourceDirectory, char *tarFileLocation, char *fileTypeExtension)
{
    char createTarCommand[1024];
    snprintf(createTarCommand, sizeof(createTarCommand), "find %s -type f -name \"*.%s\" | tar -cvf %s -T -", sourceDirectory, fileTypeExtension, tarFileLocation);

    FILE *referenceOfFile = popen(createTarCommand, "r");
    char spaceToReadData[256];
    while (fgets(spaceToReadData, sizeof(spaceToReadData), referenceOfFile) != NULL)
    {
        printf("%s", spaceToReadData);
    }

    if (pclose(referenceOfFile) == -1)
    {
        displayErrorMessage("Error while closing the file");
    }
}

// Method to check if it contains the provided extension
int doesContainProvidedExtension(char *nameOfFile, char *extension)
{
    char *positionOfDotExtension = strrchr(nameOfFile, '.');
    return (positionOfDotExtension && strcmp(positionOfDotExtension + 1, extension) == 0);
}

// Method to resolve the folder address for the test server
char *convertPathNameFromSmainToSTextServer(char *directoryPathName, char *contentToReplaceWith)
{
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

    return updatedDirectoryPathName;
}
// Method to crete dir and sub dir
void createSubDirectoriesAndFiles(char *pathOfDirectory, char *fileExtension)
{
    struct dirent *entry;
    struct stat path_stat;
    char folderLocation[1024];
    DIR *directoryReference = opendir(pathOfDirectory);

    while ((entry = readdir(directoryReference)))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(folderLocation, sizeof(folderLocation), "%s/%s", pathOfDirectory, entry->d_name);

        if (stat(folderLocation, &path_stat) == -1)
        {
            displayErrorMessage("Error in the file stat");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode))
        {
            createSubDirectoriesAndFiles(folderLocation, fileExtension);
        }
        else if (S_ISREG(path_stat.st_mode))
        {
            if (doesContainProvidedExtension(entry->d_name, fileExtension))
            {
                char *directoryPathName = dirname(folderLocation);
                char *sourceDirectoryPathName = "";
                if (strstr(directoryPathName, "smain/") != NULL)
                    sourceDirectoryPathName = convertPathNameFromSmainToSTextServer(directoryPathName, "stext/");
                else
                    sourceDirectoryPathName = convertPathNameFromSmainToSTextServer(directoryPathName, "stext");
                createFileAndDirectoryForClient(entry->d_name, sourceDirectoryPathName);
            }
        }
    }

    closedir(directoryReference);
}

// Method to create all the folders  for the provided path
void createAllFoldersForTheProvidedPath(char *providedlocation)
{
    char prepareFolderPath[800];
    char backupOfPrepareFolderPath[800];
    char *token;
    char *backupOfProvidedlocation = strdup(providedlocation);
    snprintf(prepareFolderPath, sizeof(prepareFolderPath), "%s", "/");
    snprintf(backupOfPrepareFolderPath, sizeof(backupOfPrepareFolderPath), "%s", prepareFolderPath);
    token = strtok(backupOfProvidedlocation, "/");
    while (token != NULL)
    {
        snprintf(prepareFolderPath, sizeof(prepareFolderPath), "%s%s/", backupOfPrepareFolderPath, token);
        mkdir(prepareFolderPath, 0755);
        snprintf(backupOfPrepareFolderPath, sizeof(backupOfPrepareFolderPath), "%s", prepareFolderPath);
        token = strtok(NULL, "/");
    }
}

// Method to create dir for the client
void createFileAndDirectoryForClient(char *nameOfFile, char *directoryPath)
{
    createAllFoldersForTheProvidedPath(directoryPath);
    char completePathOfFile[256];
    snprintf(completePathOfFile, sizeof(completePathOfFile), "%s/%s", directoryPath, nameOfFile);
    FILE *file = fopen(completePathOfFile, "w");
}

// <ethod to handle display command
void handleDisplayFiles(char *userCommand, char *directoryPath, int clientSocketReference)
{
    char *completeDirectoryPath = resolveDirectoryPath(directoryPath, 1);
    createSubDirectoriesAndFiles(completeDirectoryPath, "txt");
}

// Method tp resolve the dir path
char *resolveDirectoryPath(char *destinationFilePath, int isDestinationDir)
{
    char userWorkingDirectory[1024];
    char static directoryOfFile[1024];
    char static completePathOfFile[1024];
    char filePathAfterRemovingSmain[256];
    if (getcwd(userWorkingDirectory, sizeof(userWorkingDirectory)) != NULL)
    {
        int length = strlen("~smain");
        char *start = strchr(destinationFilePath, '~');
        strncpy(filePathAfterRemovingSmain, start + length, sizeof(filePathAfterRemovingSmain) - 1);

        if (filePathAfterRemovingSmain != '\0')
        {
            snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s%s/", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
            if (!isDestinationDir)
                snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s%s", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
        }
        else
        {
            snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s/%s", userWorkingDirectory, "stext", filePathAfterRemovingSmain);
        }
    }
    return directoryOfFile;
}

// Method to remove the file form the pdf server
void removeFileFromPDFServer(char *sourceFile, int clientSocketReference)
{
    char *nameOfFile = basename(sourceFile);
    char *fileToDelete = resolveDestinationPath(sourceFile, nameOfFile, 0);
    if (remove(fileToDelete) == 0)
    {
        displayMessage("Requested file has been deleted sucessfully.");
    }
    else
    {
        displayErrorMessage("Error happned while deleting the file");
    }
}

// method to create the socket
int createSocketWithPDFandTextServer(int portNumber)
{
    int socketReference;
    struct sockaddr_in server_addr;
    socketReference = socket(AF_INET, SOCK_STREAM, 0); // creating the socket

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNumber);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(socketReference, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        displayErrorMessage("Error happned while making the connection.");
        close(socketReference);
    }

    return socketReference;
}

// Method to send the downloaded file to the main server
void sendDownloadedFileFromPDFToClient(char *sourceFile, int clientSocketReference)
{
    char *nameOfFile = basename(sourceFile);
    char *fileToRead = resolveDestinationPath(sourceFile, nameOfFile, 0);
    int noOfBytesRead;
    char spaceToReadData[1024];
    FILE *referenceOfFile = fopen(fileToRead, "rb");
    while ((noOfBytesRead = fread(spaceToReadData, 1, sizeof(spaceToReadData), referenceOfFile)) > 0)
    {
        send(clientSocketReference, spaceToReadData, noOfBytesRead, 0);
    }
    displayMessage("Requested file content has been sent sucessfully.");
    fclose(referenceOfFile);
}

// Method to download file form the server
void downloadFileFromServer(char *destinationFilePath, char *sourceFileName, int clientSocketReference)
{

    char userWorkingDirectory[1024];
    char directoryOfFile[1024];
    char *nameOfFile = basename(sourceFileName);
    char *completePathOfFile = resolveDestinationPath(destinationFilePath, nameOfFile, 1);
    char *directoryPathName = dirname(completePathOfFile);
    createFileAndDirectoryForClient(nameOfFile, directoryPathName);

    snprintf(directoryOfFile, sizeof(directoryOfFile), "%s/%s", directoryPathName, nameOfFile);

    FILE *referenceOfFile;
    referenceOfFile = fopen(directoryOfFile, "wb");
    char spaceToReadData[1024];
    int noOfBytesRead;
    while ((noOfBytesRead = read(clientSocketReference, spaceToReadData, sizeof(spaceToReadData))) > 0)
    {
        fwrite(spaceToReadData, 1, noOfBytesRead, referenceOfFile);
    }

    fclose(referenceOfFile);
    displayMessage("File has been saved on the server.");
}

// Method to handle the file upload
void handleFileUpload(int clientSocketReference)
{
    char spaceToReadData[5024];

    read(clientSocketReference, spaceToReadData, 5024);

    char destinationStorage[1024];
    char sourceFile[800];
    sscanf(spaceToReadData, "%s %s", destinationStorage, sourceFile);
    char *inputFile = fixFilePath(sourceFile);
    downloadFileFromServer(destinationStorage, inputFile, clientSocketReference);
}

//Main method
int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                       // creating the socket.
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); // Setting the socket option

    address.sin_family = AF_INET; // Defining the address.
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address)); // binding the socket.
    listen(server_fd, 3);                                          // listning the connection.

    printf("Server listening on port %d\n", PORT);

    // Listen for connections and handle file uploads
    while (1)
    {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // accepting the connection.
        processCommand(new_socket);
        close(new_socket);
    }

    return 0;
}
