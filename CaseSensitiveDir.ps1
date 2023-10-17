# this command set recursively the directories to be case sensitive.
# like that all includes files who are badly writen in cpp files will give an error, like in unix

$directories = Get-ChildItem . -Recurse -Directory ForEach($dir In $directories) { fsutil.exe file setCaseSensitiveInfo $dir.FullName enable }