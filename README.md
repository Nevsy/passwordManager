# ⚠️⚠️⚠️
# THIS IS NOT SAFE
The password manager contained is DOES NOT have ANY form of encryption.

Consult a proper password manager (like the terminal based Pass) if you want a password manager. 

Additionally it is far from well structured code, and contains a tonne of things yet to improve (UX, bugs to fix, file management and placement, customisation, ...).

This is a project I have made to learn more about C, C libraries and to have some fun.
# ⚠️⚠️⚠️

# Usage
(not up to date: see "-h" or "--help")
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
(not up to date: see "-h" or "--help")

# 'Documentation'
This documentation probably isn't to the convention...
 (and might not be up to date)

- show <name>:
    - shows a password to the terminal
    - (Ideally I would like to find a better way to handle the flags, to improve the UX)
    - -m: show corresponding metadata
    - -c: copy the password to the clipboard (WINDOWS ONLY, following -c commands are exclusive to this OS too), instead of printing it to the terminal
- add <name>:
    - asks you for a password twice (hidden)
    - -c: copy entered password to clipboard _instead of printing it to the terminal_ 
    - -m <metadata>: add metadata to the file 
- gen | generate <name>:
    - generated random password
    - -m: show metadata
    - -c: copy the password to the clipboard, instead of printing it to the terminal
- edit <name>:
    - auto opens a vi instance on the specified file
    - -a <metadata>: appends new metadata line to file
    - -r <metadata>: replace all existing metadata with new line
- delete <name>:
    - auto: deletes the specified password file
    - -m: only deletes metadata
- ls:
    - lists all saved passwords
- grep <pattern>:
    - searches through the password files for a matching query
    - -i: deactivates case sensitivity
 
# ⚠️⚠️⚠️
# THIS IS NOT SAFE
(see up top)
# ⚠️⚠️⚠️
