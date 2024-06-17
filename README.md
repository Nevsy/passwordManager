# Usage
```C
printf("--> password.exe show <name> [-m | -c] \n");
printf("\tshow a password (and metadata, copy to clipboard and don't show in terminal)\n");

printf("--> password.exe add <name> [-m] <metadata> [-c] \n");
printf("\t--> add a password (and add metadata)\n");

printf("--> password.exe gen | generate <name> <metadata> \n");
printf("\t--> generate a password and add it (and add metadata)\n");

printf("--> password.exe edit <name> [-a | -r] <metadata> \n");
printf("\t--> edit a password (a: add inline, r: replace with, auto: open vim)\n");

printf("--> password.exe delete <name> [-m] \n");
printf("\t--> delete a password, or only the metadata\n");

printf("--> password.exe ls \n");
printf("\t--> list all passwords\n");
```
# Documentation
- show <name>:
    - 
    - -m
    - -c
- add <name>:
    - asks you for a password twice (hidden)
    - -c: copy entered password to clipboard
    - -m <metadata>: add metadata to the file 
- gen | generate <name>:
    - generated random password
    - same flags as add command
- edit <name>:
    - auto opens a vi instance on the specified file
    - -a <metadata>: appends new metadata line to file
    - -r <metadata>: replace all existing metadata with new line
- delete <name>:
    - auto: deletes the specified password file
    - -m: only deletes metadata
- ls:
    - lists all saved passwords
- grep:
