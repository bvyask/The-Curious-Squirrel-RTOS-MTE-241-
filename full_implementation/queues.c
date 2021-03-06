/*
 *  queueimplementation.c
 *  
 *
 *  Created by Markus Trapp on 11-11-06.
 *  Copyright 2011 University Of waterloo. All rights reserved.
 *
 */

/* Include Files and Libraries */

#include <stdio.h>
#include <stdlib.h>
#include "queues.h"

int initialize_pcb(pcb *to_be_initialized){
	if(to_be_initialized == NULL)
		return INVALID_PCB_POINTER;
	to_be_initialized->next=NULL;											//new pcbs next always points to null
	to_be_initialized->inbox= (msg_queue*)malloc(sizeof(struct msg_queue));	//allocates space for inbox message queue
	initialize_msg_queue(to_be_initialized->inbox);							//makes the inboxs head and tail point to null
	
	return 1;
}

int initialize_msg_env(Msg_Env *to_be_initialized){
	if(to_be_initialized==NULL)
		return INVALID_MSG_POINTER;
	
	to_be_initialized->next = NULL;											//new message envelope next always point to null
	msg_enqueue_all(to_be_initialized);										// add msg_env to the all envelope queue
	return 1;
}


//here are pcb specific queue functions 

void initialize_queue(pcb_queue *Q){
	if(Q == NULL)
		return;
	
  	Q->head = NULL;							//queue start with zero elements
	Q->tail = NULL;
}

int empty_pcb_queue(pcb_queue *Q){
	if(Q == NULL)
		return INVALID_QUEUE_ERROR;
	if (Q->head == NULL)
		return 1;														//queue is empty
	else
		return 0;														//false queue is not empty
}


int enqueue(pcb_queue *Q, pcb *new_pcb){
	if(Q == NULL)
		return INVALID_QUEUE_ERROR;
	if(new_pcb == NULL)
		return INVALID_PCB_POINTER;

	if(empty_pcb_queue(Q)){												//if queue is currently empty head and tail point to same element
		Q->head = new_pcb;
		Q->tail = new_pcb;
	}

	else{
		Q->tail->next = new_pcb;										//pcb who is currently is at end now points to the new pcb
		Q->tail = new_pcb;												//tail points to new pcb
	}
	new_pcb->next = NULL;										//ensures pcb next is pointing to nothing else
	return 1;	
	
}


//this will take first element out of the queue and return pointer to said element
pcb *dequeue(pcb_queue *Q){
	if(Q == NULL)
		return NULL;
	if(Q->head == NULL && Q->tail == NULL)
		return NULL;
	else{
		pcb *front= Q->head;
		Q->head = Q->head->next;
		
		if(Q->head == NULL)								//if the last item is being dequeued
			Q->tail = NULL;
		
		front->next = NULL;
		return front;
	}
}

//this dequeue remove a specified process from the queue, pid of process which is to be removed is passed to this function
pcb *dequeue_selected_pcb(pcb_queue *Q, int desired_pcb){				
	if(Q == NULL)
		return NULL;
	if(empty_pcb_queue(Q))
		return NULL;
	if(desired_pcb >9 || desired_pcb<0 )
		return NULL;
	if(Q->head == NULL && Q->tail == NULL)
		return NULL;
	pcb *current_pcb = Q->head;
	pcb *previous_pcb;
	int pcb_found=0;

	
	if(current_pcb->pid == desired_pcb)    // if desired pcb is the first element
		current_pcb=dequeue(Q);
	else{

		while (current_pcb != NULL && pcb_found==0){							//loops until found or looped throught the entire queue
	
			if(current_pcb->pid != desired_pcb){
				previous_pcb = current_pcb;
				current_pcb = current_pcb->next;
			}

			else{
				pcb_found=1;
				previous_pcb->next = current_pcb->next;							//the one behind the one we remove point to the pcb after (jumps it)
				if(current_pcb->next == NULL)									//if the pcb we are looking for is the last pcb
					Q->tail=previous_pcb;										//sets tail to second last pcb
				current_pcb->next = NULL;										//current_pcb no longer looks at anything
			}	
		}
	}
	return current_pcb;
	
}

/*//this function will delete (free all memory) of a desird queue
int delete_pcb_queue (pcb_queue *Q){
	pcb *current_pcb = Q->head;
	pcb *next_pcb;
	
	while(current_pcb != NULL){						//will loop until all elements of the queue are freed
		next_pcb = current_pcb;						//stores the next value of current_pcb so we dont lose the queue
		free(current_pcb);							//frees the pcb we are currently looking at
		current_pcb;
	}
	
	free(Q);										//frees the memory of the actual 
	
}*/

//this will return the pointer of a pcb specified by the id passed in (looks through the all_pcb_queue)
pcb* pcb_pointer (int desired_pcb){
	if (pcbList[desired_pcb] != NULL){
		return pcbList[desired_pcb];
	}
	return NULL;
}

/************* here is where message envelope queue functions begin*******************/

int initialize_msg_queue(msg_queue *Q){
	if(Q == NULL)
		return 0;
	Q->head = NULL;	//queue start with zero elements
	Q->tail = NULL;
	return 1;
}

int empty_msg_queue(msg_queue *Q){
	if(Q == NULL){
		return INVALID_QUEUE_ERROR;
	}

	if (Q->head == NULL){
		return 1;	//queue is empty
	}
	
	else{
		return 0;  //false queue is not empty
	}
}

//add message envelopes to the queue which contains all envelopes used once
int msg_enqueue_all (Msg_Env *chain_mail){
	if(all_envelopes == NULL)
		return INVALID_QUEUE_ERROR;
	if(chain_mail == NULL)
		return INVALID_MSG_POINTER;

	if(empty_msg_queue(all_envelopes)==1){						//if queue is currently empty head and tail point to same element
		all_envelopes->head = chain_mail;
		all_envelopes->tail = chain_mail;
	}
	
	else{
		chain_mail->env_all = NULL;
		all_envelopes->tail->env_all = chain_mail;				//message envelope who is currently is at end now points to the new message envelope
		all_envelopes->tail = chain_mail;					//tail points to new message envelope
	}
	
	chain_mail->env_all=NULL;
	return 1;	
}

int msg_enqueue(msg_queue *Q, Msg_Env *chain_mail){
	if(Q == NULL)
		return INVALID_QUEUE_ERROR;
	if(chain_mail == NULL)
		return INVALID_MSG_POINTER;
	
	if(chain_mail->message_type == M_TYPE_MSG_DELAY){
		Msg_Env *previous_message, *current_message;
		
		if(Q->head == NULL){										//if the queue is empty
			Q->head = chain_mail;
			Q->tail = chain_mail;
		}
		
		else if(Q->head->next == NULL && Q->head->time_stamp < chain_mail->time_stamp){
			Q->head->next = chain_mail;
			Q->tail= chain_mail;
		}
		else if(Q->head->next == NULL && Q->head->time_stamp >= chain_mail->time_stamp){
				chain_mail->next = Q->head;
				Q->head = chain_mail;
				Q->tail = chain_mail->next;
		}
		
			
		else if(Q->head->time_stamp >= chain_mail->time_stamp){	//if there is one element in queue or first element is larger then new element
			chain_mail->next = Q->head;
			Q->head = chain_mail;
			
		}


		else if(Q->tail->time_stamp < chain_mail->time_stamp){
			Q->tail->next= chain_mail;
			Q->tail = chain_mail;
		}
		else{
			current_message= Q->head->next;
			previous_message = Q->head;
			while(current_message != NULL){
				if(chain_mail->time_stamp > current_message->time_stamp){
					previous_message = current_message;
					current_message=current_message->next;
				}
				else{
					chain_mail->next = current_message;
					previous_message->next = chain_mail;
					current_message = NULL;
				}
				
					
					
			
				}
			}			
		
		
	}\
	else{
		if(empty_msg_queue(Q)){					//if queue is currently empty head and tail point to same element
				Q->head = chain_mail;
				Q->tail = chain_mail;
		}
	
		else{
			chain_mail->next = NULL;
			Q->tail->next = chain_mail;		//message envelope who is currently is at end now points to the new message envelope
			Q->tail = chain_mail;			//tail points to new message envelope
			}
		
			chain_mail->next=NULL;
	}
	
		return 1;	
}


//this will take first element out of the queue and return pointer to said element
Msg_Env *msg_dequeue(msg_queue *Q){
	if(Q == NULL)
		return NULL;
	if(Q->head == NULL && Q->tail == NULL)
		return NULL;
	else{
		Msg_Env *front= Q->head;
		Q->head = Q->head->next;
		
		if(Q->head == NULL)
			Q->tail = NULL;										//if the last item is being dequeued
		
		front->next = NULL;
		return front;
	}	

}

//this function will delete (free all memory) of a desird Msg_env ONLY USED WHEN TERMINATING RTX
/*int delete_msg_env (Msg_Env *message){
	if (message== NULL)
		return INVALID_MSG_POINTER;

	if (message->next != NULL)									//cant delete if it exists in a queue other than all msg env queue
		return INVALID_MESSAGE_DELETE;

		free(message->owner_id);
		free(message->sender_id);
		free(message->time_stamp);
		free(message->message_type);
		free(message->flag);
		//free(message->message);
		free(message);											//frees the message  we are currently looking at

	return 1;
}*/
//this function will delete (free all memory) of a desird Msg_env ONLY USED WHEN TERMINATING RTX
/*int delete_msg_all(msg_queue *Q)
{
	if (Q== NULL)
		return INVALID_QUEUE_ERROR;
	
	Msg_Env *current, *next;
	current=Q->head;

	while(current!=NULL){
		next = current->next;
		
		if(delete_msg_env(current)==INVALID_MESSAGE_DELETE)
			return INVALID_MESSAGE_DELETE;
		else
			current=next;
	}
	free(Q);
	
	return 1;
}*/

/***************here is where the code for the ready process queue starts************************************/

int initialize_rpq_queue(){

	int i;
	for(i=0;i<4;i++){
		priority_ready_queue[i] = (pcb_queue*)malloc(sizeof(struct pcb_queue));
		priority_ready_queue[i]->head = NULL;	//queue start with zero elements
		priority_ready_queue[i]->tail = NULL;
	}
	return 1;
}

int rpq_enqueue (pcb *ready_pcb){
	int i;
	if (ready_pcb == NULL)
		return INVALID_PCB_POINTER;	
	if(ready_pcb->pid >9 || ready_pcb->pid<0)
		return INVALID_PID_ERROR;
	if (ready_pcb ->state != READY)
		return INVALID_PCB_STATE_ERROR;
	
	i = ready_pcb->priority;

	ready_pcb->next= NULL;
	if(priority_ready_queue[i]->head == NULL && priority_ready_queue[i]->tail ==NULL ){		//if queue is currently empty head and tail point to same element
		priority_ready_queue[i]->head = ready_pcb;
		priority_ready_queue[i]->tail = ready_pcb;
	}
	else{
		priority_ready_queue[i]->tail->next = ready_pcb;    //pcb who is currently is at end now points to the new pcb
		priority_ready_queue[i]->tail = ready_pcb;   //tail points to new pcb
		priority_ready_queue[i]->tail->next = NULL;
	}

	return 1;	
}



pcb* rpq_dequeue (){
	int i=0;
	while(priority_ready_queue[i]->head == NULL && priority_ready_queue[i]->tail == NULL){						//loops until a queue level is found whch contains elements
		i++;
	}
		
	return dequeue(priority_ready_queue[i]);
}


pcb* rpq_dequeue_specific_pcb (pcb *desired_pcb){											//only needed if changing the priority of a process already in the rpq
	pcb *specified_pcb;
		
	specified_pcb = dequeue_selected_pcb(priority_ready_queue[desired_pcb->priority], desired_pcb->pid);

	return specified_pcb;
}

/***************  MESSAGE TRACE BUFFER Stuff***************************/
int initialize_msg_trace(msg_trace_buffer *trace){
	if(trace == NULL)
		return INVALID_TRACE_BUFFER_PTR;
	if(trace->messages == NULL)
		return INVALID_TRACE_PTR;
	int i;
	for(i=0; i<16;i++){
		trace->messages[i]=(msg_trace*)malloc(sizeof(struct msg_trace));
		trace->messages[i]->sender_PID = -1;
		trace->messages[i]->receiver_PID = -1;
		trace->messages[i]->time_stamp = -1;
		trace->messages[i]->m_type = -1;
	}

	return 1;
}


int enqueue_msg_trace(msg_trace_buffer *trace ,Msg_Env *message){
	int i;
	if(message==NULL)
		return INVALID_MSG_POINTER;

	//shifts all the data field up one space if it is full	
	if (trace->entry_element > 15){
		for (i = 1; i < 16; i++){
			trace->messages[i-1]->sender_PID = trace->messages[i]->sender_PID;
			trace->messages[i-1]->receiver_PID = trace->messages[i]->receiver_PID;
			trace->messages[i-1]->time_stamp = trace->messages[i]->time_stamp;
			trace->messages[i-1]->m_type = trace->messages[i]->m_type;
		}
		//set the entry_element so that it saves it to the last space.
		trace->entry_element = 15;
	}

	trace->messages[trace->entry_element]->sender_PID= message->sender_id;
	trace->messages[trace->entry_element]->receiver_PID = message->owner_id;				//receiver id
	trace->messages[trace->entry_element]->time_stamp = time_since_init;	
	trace->messages[trace->entry_element]->m_type = message->message_type;
	
	trace->entry_element+=1;
	number_messages_sent +=1;
	
	return 1;
}
