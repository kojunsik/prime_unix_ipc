#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "prime_common.h"
#include "prime_files.h"

void signal_handler(int signal) {
	if (signal == SIGINT) {
		printf("\nCtrl+C pressed. Cleaning up...\n");
		cleanup();
		exit(0); 
	}
}

int main(void) {
	struct primemsgbuf inmsg = { 0 };
	key_t key;
	int msgid;

	int len;
  
  signal(SIGINT, signal_handler);

	key = ftok(KEYPATH, PROJID);
	if ((msgid = msgget(key, IPC_CREAT | 0644)) < 0) {
		perror("msgget");
		exit(1);
	}

	startup();

	/* LOOP */
	while (1) {
		len = msgrcv(msgid, &inmsg, PBUFSIZE, 0, 0); //msgtype, msgflag

		for (int i = 0; i < ARGSIZE; i++)
			printf("%lld ", inmsg.args[i]);
		
		struct primemsgbuf outmsg = { MTYP_RES, inmsg.cmd, { 0 } };
		primenum_t res = 0;
		bool should_reply = false;

		switch (inmsg.cmd) {
		case CMD_CHECK_NUM:
			puts("CHECK_NUM");
			res = check_num(inmsg.args[0]);
			should_reply = true;

			break;

		case CMD_ALLOC_ONE:
			puts("ALLOC_ONE");
			res = alloc_one();
			should_reply = true;
		
			break; 

		case CMD_SET_ONE:
			puts("SET_ONE");
			set_one(inmsg.args[0], inmsg.args[1]);
			should_reply = false;

			break;

		default:
			puts("ERROR: got unknown cmd");
			break;
		}

		//ret.
		if (should_reply) {
			outmsg.args[0] = res;

			if (msgsnd(msgid, (void *) &outmsg, PBUFSIZE, IPC_NOWAIT) == -1) {
				perror("msgsnd");
				exit(1);
			}
		}
	}

	cleanup(); //TODO: when interrupted or killed, make sure this is activated using signal catching
	return 0;
}