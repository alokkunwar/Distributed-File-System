# Distributed File System using Socket Programming

## Project Overview

This project implements a simple distributed file system using socket programming in C. The system consists of three servers (`Smain`, `Spdf`, and `Stext`) and multiple clients. The main server (`Smain`) handles all client requests, while the `Spdf` and `Stext` servers handle `.pdf` and `.txt` files respectively. Clients interact solely with the `Smain` server and are unaware of the existence of `Spdf` and `Stext`.

## Project Structure

- **Smain.c**: The main server that handles all client requests and delegates tasks related to `.pdf` and `.txt` files to the appropriate servers (`Spdf` and `Stext`).
- **Spdf.c**: The server responsible for handling `.pdf` files. It communicates with `Smain` to store, retrieve, or delete PDF files.
- **Stext.c**: The server responsible for handling `.txt` files. It communicates with `Smain` to store, retrieve, or delete text files.
- **client24s.c**: The client program that connects to `Smain` to perform file operations like uploading, downloading, and deleting files.

## Features

- **ufile**: Upload a file to the distributed file system.
- **dfile**: Download a file from the distributed file system.
- **rmfile**: Delete a file from the distributed file system.
- **dtar**: Download all files of a certain type as a tarball.
- **display**: Display the list of files in a specified directory.

## Requirements

- A C compiler (e.g., `gcc`)
- Multiple terminals or machines to simulate different servers and clients
- Basic knowledge of socket programming and inter-process communication

## Installation

1. Clone the repository:
    ```bash
    git clone https://github.com/alokkunwar/Distributed-File-System.git
    cd Distributed-File-System
    ```

2. Compile the source files:
    ```bash
    gcc Smain.c -o Smain
    gcc Spdf.c -o Spdf
    gcc Stext.c -o Stext
    gcc client24s.c -o client24s
    ```

## Running the Servers and Client

1. **Start the Main Server (`Smain`)**:
    - Open a terminal and run:
      ```bash
      ./Smain
      ```

2. **Start the PDF Server (`Spdf`)**:
    - Open a second terminal and run:
      ```bash
      ./Spdf
      ```

3. **Start the TXT Server (`Stext`)**:
    - Open a third terminal and run:
      ```bash
      ./Stext
      ```

4. **Run the Client (`client24s`)**:
    - Open a fourth terminal and run:
      ```bash
      ./client24s
      ```

## Example Usage

1. **Upload a `.c` file to `Smain`:**
    ```bash
    Enter command: ufile Alok.c ~/smain/folder1/folder2
    ```

2. **Upload a `.pdf` file (handled by `Spdf`):**
    ```bash
    Enter command: ufile Alok.pdf ~/smain/folder1/folder2
    ```

3. **Upload a `.txt` file (handled by `Stext`):**
    ```bash
    Enter command: ufile Alok.txt ~/smain/folder1/folder2
    ```

4. **Download a file:**
    ```bash
    Enter command: dfile ~/smain/folder1/folder2/Alok.c
    ```

5. **Delete a file:**
    ```bash
    Enter command: rmfile ~/smain/folder1/folder2/Alok.pdf
    ```

6. **Display files in a directory:**
    ```bash
    Enter command: display ~/smain/folder1/folder2
    ```

## Notes

- Ensure that each server (`Smain`, `Spdf`, `Stext`) is running on a separate terminal or machine.
- The client can only interact with `Smain` and is unaware of `Spdf` and `Stext`.
- Error handling is basic and should be improved as the project evolves.
- Future enhancements include implementing the full logic for file storage, retrieval, and deletion.
