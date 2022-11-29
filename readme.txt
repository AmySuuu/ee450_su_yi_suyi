Full Name: Yi Su
Student ID: 6838-0352-18

I didn't do the optional part of this project

client.cpp: the client side server
serverM.cpp: the main server
serverC.cpp: the credential server
serverCS.cpp: the Computer Science department server
serverEE.cpp: the Electrical Engineering department server
Makefile: makefile of this project
readme.txt: all information about the project

No idosyncrasy so far

I used code from Beej's tutorials 

Problems I met during the project
Problem 1
when trying to compare two string whether they are equal, though they are equal, but they are not the same
strcmp(cred_recv_buf, "0")
serverc_send_result = sendto(serverc_udp_socket, auth_result, 1, 0, (struct sockaddr *) &their_addr, addr_len);
I set the length rather than 1024 to 1, then the authentication is successful

Problem 2
When entered the wrong username, it won't jump to the wrong else loop. I found I need to have one more condition
to ask input from user

Problem 3
When trying to see if the password match, it always return password not match, I found that in the end 
of each cred.txt, there might be a hidden control character, so I filter them and then the comparison work