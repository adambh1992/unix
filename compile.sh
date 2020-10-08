gcc telnet.c -lcli -lpthread -o telnet.o  -c 
gcc -g  main.c telnet.o -lcli -lpthread  -finstrument-functions -ldl -o myFileSystemMonitor 