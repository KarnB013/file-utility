# File utility
A simple program to search, copy or move a file

To run _fileutil_ instead of _./fileutil_, add program location to PATH environment variable

Usage (in case of 3 arguments):

fileutil _root_directory filename_ : Searches for file in _root directory_ and lists location for first occurence

Usage (in case of 4 arguments):

fileutil _root_directory storage_directory extension_ : Searches for file/s matching the extension in _root directory_ and creates a .tar archive of them, which is then stored inside _storage directory_

Usage (in case of 5 arguments):

fileutil _root_directory storage_directory option filename_ : Searches for file in _root directory_ and based on the _option_ (_-cp_ or _-mv_), copies or moves the file to _storage directory_
