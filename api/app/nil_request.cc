#include <stdlib.h>
#include <node.h>

#include "nil.h"




void initRequestList(RequestList *rList){
	rList->rq_num = -1;
	rList->next = NULL;
}

void printRequset(RequestList *rList){
	printf("============ Requset List ============ \n");
	
	while(1){
		if(rList == NULL)
			break;

		printf("Request Number : %3d  \n", rList->rq_num);


		rList = rList->next;
	}
	printf("=======================================\n");

}
int countRequest(RequestList *rList){
	int count = 0;

	for(;;){
		if(rList == NULL)
			return count;
		
		else{
			count++;
			rList = rList->next;
		}	
	}
}

RequestList * newRequest(RequestList *rList){
	RequestList *rl = rList;

	if (rList->rq_num == -1){
		rList->rq_num = 0;
		return rList;
	}
	
	for (;;){
		if (rList->next == NULL)
			break;
		rList = rList->next;
	}

//	rList->next = (RequestList*)malloc(sizeof(RequestList));
	rList->next = new RequestList();
	rList->next->next = NULL;
	rList->next->rq_num = rList->rq_num + 1;

	return rList->next;
}


RequestList * getRequest(RequestList *rList, int rq_num){
	RequestList *rl;

	rl = rList;
	for (;;){
		if (rl->rq_num == rq_num){
			printf("[NIL] Found Request \n");
			break;
		}
		rl = rl->next;
		if (rl == NULL){
			return NULL;
		}
	}

	return rl;
}

#define DELETE_REQUEST 0
#define DELETE_FAIL_NOT_EXIST 1

int deleteRequest(RequestList *rList, int rq_num){
	RequestList *rl;
	RequestList *del_rl = NULL;
	RequestList *prev_rl = NULL;

	rl = rList;


	if (rList->rq_num == rq_num){
		//root is target.
		del_rl = rList->next;
		memcpy(rList, rList->next, sizeof(RequestList));
		free(del_rl);

		return DELETE_REQUEST;
	}
	for (;;){
		if (rl->rq_num == rq_num)
			break;

		prev_rl = rl;
		rl = rl->next;
		
		if (rl == NULL){
			return DELETE_FAIL_NOT_EXIST;
		}
	}

	prev_rl->next = rl->next;
	free(rl);

	return DELETE_REQUEST;

	
}
