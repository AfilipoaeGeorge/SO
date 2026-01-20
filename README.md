🛡️ **File System Monitor & Malware Detection System**

📌 **Description:**
This project implements a file system monitoring system capable of detecting directory and file changes, as well as potential malware files, based on predefined heuristics.
The solution uses multi-process execution, inter-process communication via pipes, and an auxiliary Bash script for file analysis.

👤 **Author & Project Information:**
- Author: Afilipoae George-Marian
- Course: Operating Systems – Laboratory
- Programming Language: C
- Auxiliary Technologies: Bash, POSIX API

⚙️ **Main Features:**
- 🔁 Recursive directory monitoring
- 📸 Snapshot creation and comparison for change detection
- 🚨 Malware detection for files with 000 permissions
- 🔒 Automatic isolation of dangerous files
- ⚡ Parallel processing using multiple processes
- 🔗 Inter-process communication using pipes
- 👶 Grandchild processes for file analysis

🧩 **System Requirements:**
- Operating System: Linux / Unix
- Compiler: GCC
- Shell: Bash
- Libraries: sys/types.h, sys/stat.h, dirent.h, unistd.h, fcntl.h, string.h

🛠️ **Installation & Compilation:**
- gcc -Wall -o p proiectLaboratorSO.c
- chmod +x verify_for_malicious.sh

▶️ **Usage:**
- ./p -o dirOutput -s dirIsolation dir1 dir2 dir3 ...

Parameters:
- -o dirOutput – directory where snapshots are stored
- -s dirIsolation – directory where dangerous files are moved
- dir1 dir2 ... – directories to be monitored (maximum 10)

🔍 **Program Workflow:**
- Recursively traverses the specified directories
- Collects file metadata: file name, permissions, size, last modification time
- Creates and compares snapshots
- Detects: added or deleted files, permission changes, size or timestamp modifications
- Analyzes files with 000 permissions using a Bash script
- Moves dangerous files to the isolation directory

🧪 **Malware Detection Criteria: The verify_for_malicious.sh script flags a file as malicious if:**
- it has fewer than 3 lines
- contains more than 1000 words
- exceeds 2000 characters
- includes non-ASCII characters
- contains the following keywords:
- corrupted, dangerous, risk, attack, malware, malicious

📤 **(Output)
The program provides:**
- messages about process creation and termination
- detected directory and file changes
- added or removed files
- files moved to the isolation directory
- a snapshot.txt file containing snapshot details

🧠 **Technical Highlights:**
- parallel processing using fork()
- inter-process communication with pipe()
- child and grandchild processes
- error handling and permission checks
- automatic creation of output and isolation directories

⚠️ **Limitations:**
- maximum 100 metadata entries per directory
- maximum 10 directories processed simultaneously
- supported only on Linux / Unix systems

📜 **License:**
This project is intended for educational purposes only, developed as part of the Operating Systems laboratory.

📝 **Final Notes:**
This project demonstrates core operating system concepts, including:
- process management
- inter-process communication
- file system operations
- synchronization and isolation mechanisms
