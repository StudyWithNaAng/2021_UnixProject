/*
	 �ؾ��ϴ� ��
	1. alarm handler �Լ� �����
	2. 

	 �ð� ������?
	1. ���� �繰�� -> part ������
	2. ��й�ȣ �ڸ��� 5�ڸ� �̻�
	3. �θ𿡼� fork
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/socket.h>
#include <pthread.h>
#include "locker.h"

#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
#define MAX_LOCKER_SIZE 50
;

//�繰�� ����
int locker_size;
int **locker_info;
struct locker *locker;

char message[200];

int clnt_socks[10]; //�ִ� ���� ������ client ��
int clnt_cnt = 0; //���� ������ client ��
int locker_num, locker_use, locker_conn;
char locker_pw[10];

pthread_mutex_t mutx;

char* client_pw_setting(int cfd, char* message)
{
	//setting and recv
	static char client_pw[10];
	send(cfd, &message, sizeof(message), 0);
	recv(cfd, &client_pw, sizeof(client_pw), 0);
	printf("message is %s\n", message);
	printf("client_pw is %s\n", client_pw);
	
	return client_pw;
}

void client_show_locker(int cfd)
{
	printf("in client_show_locker\n");
	
	int i;
	char message[400];
	char message1[20];

	sprintf(message, ("| --------------------���� �繰���� ����-------------------- |\n"
			"������� : 0, ����� : 1\n"));
	for(i=1; i<locker_size+1; i++)
	{
		if((i-1)%5 == 0)
		{
			strcat(message, "\n|");
		}
		sprintf(message1, " [%d]\t%d\t|" , i, locker[i].use);
		strcat(message, message1);
	}
	strcat(message, "\n");
	send(cfd, &message, sizeof(message), 0);
	printf("message is %s\n", message);
}

int client_1_action(int cfd)
{
	int action_num;
	char message[200];

	sprintf(message, ("welcome\n"
			 "please choose what you want\n"
			 "\t1: �繰�Կ� ���� �ֱ�\n"
			 "\t2: �繰�Կ��� ���� ã��\n"
			 "���ڸ� �Է��ϼ���: "));
	send(cfd, &message, sizeof(message), 0);
	read(cfd, &action_num, sizeof(action_num));
	
	return action_num;
}

int client_choose_locker(int cfd)
{
	sprintf(message, "\n����ϱ⸦ ���ϴ� �繰���� ��ȣ�� �Է��ϼ��� : ");
	send(cfd, &message, sizeof(message), 0);
	read(cfd, &locker_num, sizeof(locker_num));
	
	return locker_num;
}


void *t_function(void *data)
{
	int cfd;
	int action_num;
	
	cfd = *((int *)data); //cfd

	printf("in function\n");
	

	//1. welcome + action_num �ޱ�
	sprintf(message, ("welcome\n"
			 "please choose what you want\n"
			 "\t1: �繰�Կ� ���� �ֱ�\n"
			 "\t2: �繰�Կ��� ���� ã��\n"
			 "���ڸ� �Է��ϼ���: "));
	send(cfd, &message, sizeof(message), 0);
	read(cfd, &action_num, sizeof(action_num));
	
	//2. ���� �繰���� ���� �˷���
	client_show_locker(cfd);
	
	if (action_num == 1) //�繰�Կ� ���� �ֱ�
	{
		//3. ���ϴ� �繰�Թ�ȣ �Է¹ޱ�
		sprintf(message, "\n����ϱ⸦ ���ϴ� �繰���� ��ȣ�� �Է��ϼ��� : ");
		send(cfd, &message, sizeof(message), 0);
		read(cfd, &locker_num, sizeof(locker_num));

		locker_use = locker[locker_num].use;
		locker_conn = locker[locker_num].conn;
		write(cfd, &locker_use, sizeof(locker_use));
		write(cfd, &locker_conn, sizeof(locker_conn));
		//printf("@@recv locker_num: %d, locker_use: %d\n", locker_num, locker_use);

		//4. �繰���� ����ִ���, �������� �ƴ��� Ȯ��
		if ((locker_use == 0) && (locker_conn == 0))
		{
			char ans;
			char client_pw[10], client_cpw[10];
			
			locker[locker_num].conn = 1;
			sprintf(message, "[%d]�� �繰���� ������ϴ�. ����Ͻðڽ��ϱ�?(Y/N) ", locker_num);
			send(cfd, &message, sizeof(message), 0);
			read(cfd, &ans, sizeof(ans));
			 
			if (ans == 'Y')
			{		 	
			 	locker[locker_num].use = 1;
	 	
	 			//5. ��й�ȣ ����
			 	sprintf(message, "������ ��й�ȣ�� �Է����ּ���: ");
			 	send(cfd, &message, sizeof(message), 0);
			 	recv(cfd, &client_pw, sizeof(client_pw), 0);
			 	printf("client's pw : %s\n", client_pw);
			 	
			 	sprintf(message, "�ٽ� �Է����ּ���: ");
			 	send(cfd, &message, sizeof(message), 0);
			 	recv(cfd, &client_cpw, sizeof(client_cpw), 0);
			 	
			 	strcpy(locker[locker_num].pw, client_pw);
			 	
			 	
			 	
			 	printf("locker idx : %d, use : %d, conn : %d, pw : %s\n", locker_num, locker[locker_num].use, locker[locker_num].conn, locker[locker_num].pw);
			 	
			 	locker[locker_num].conn = 0; //�� ��������� ���� ����

				printf("locker idx : %d, use : %d, conn : %d, pw : %s\n", locker_num, locker[locker_num].use, locker[locker_num].conn, locker[locker_num].pw);
			 }
			 
			 else if (ans == 'N')
			 {
			 	locker[locker_num].conn = 0;
			 	sprintf(message, "�����մϴ�. �ȳ��� ������ :)\n");
			 	send(cfd, &message, sizeof(message), 0);
			 	printf("�����մϴ�. �ȳ��� ������ :)\n");
			 }
		 
		}
		else if (locker_use == 1)
		{
			 sprintf(message, "����ִ� �繰���� �������ּ���.\n");
			 send(cfd, &message, sizeof(message), 0);
			 printf("����ִ� �繰���� �������ּ���.\n");
		}
		else if (locker_conn == 1)
		{
			sprintf(message, "�ٸ� ����ڰ� �ش� �繰�Կ� �����ϰ� �ֽ��ϴ�. ��� �Ŀ� �õ����ּ���.\n");
			 send(cfd, &message, sizeof(message), 0);
			 printf("�ٸ� ����ڰ� �ش� �繰�Կ� �����ϰ� �ֽ��ϴ�. ��� �Ŀ� �õ����ּ���.\n");
		}
	}
	else if (action_num == 2) //�繰�Կ��� ���� ã��
	{	
		int check;
		char client_pw[10]; 


		//printf("B[1] num : %d, B pw : %s\n", locker[1].num, B[1].pw);
		//printf("B[2] num : %d, B pw : %s\n", B[2].num, B[2].pw);


		sprintf(message,"�繰�� ��ȣ�� �Է��ϼ���: ");
		send(cfd, &message, sizeof(message), 0);
		locker[locker_num].conn = 1; //���� ��� �Ұ��ϵ���?

		recv(cfd, &locker_num, sizeof(locker_num), 0);
		printf("locker_num is %d\n", locker_num);

		// locker ��� ���� �Ǵ� (0: empty, 1: full)
		locker_use = locker[locker_num].use;
		send(cfd, &locker_use, sizeof(locker_use), 0);

		if (locker_use == 0)
		{
			 sprintf(message, "����ִ� �繰���Դϴ�.\n������ ������� �繰���� �������ּ���\n");
			 send(cfd, &message, sizeof(message), 0);

		}
		else if (locker_use == 1)
		{
		 //��й�ȣ�� �Է��ϼ���
		 	send(cfd, &locker[locker_num].pw, sizeof(locker[locker_num].pw),0);
			 sprintf(message, "��й�ȣ�� �Է��ϼ���: ");
			 send(cfd, &message, sizeof(message), 0);

			 // recv ��й�ȣ
			 recv(cfd, &client_pw, sizeof(client_pw), 0);
			 
			 int i=1;

			 while(i < 6)
			 {
				//recv(cfd, &i, sizeof(message), 0);
				strcpy(locker_pw, locker[locker_num].pw);
		
				//check = strcmp(client_pw, locker_pw);
				if(!strcmp(client_pw, locker_pw)) //strcmp ������ 0 
				{
					sprintf(message, "������ּż� �����մϴ�.\n");
				 	send(cfd, &message, sizeof(message), 0);
				        locker[locker_num].use = 0;
				        locker[locker_num].conn = 0;
				        i=6;
					
			    	}
				else
				{     				  
				     	if (i ==5)
				     	{
						sprintf(message, "�߸��� ��й�ȣ�Դϴ�. 1�е��� ���α׷��� ���ϴ�.\n");
						send(cfd, &message, sizeof(message), 0);
						locker[locker_num].conn = 0;
				     //signal(SIGALRM, alarmHandler);
				     //alarm(5);   
			      		}
					else
			     		{
						sprintf(message, "[%d��° �õ�] ��й�ȣ�� �ٽ� �Է��ϼ���: ",i);
						send(cfd, &message, sizeof(message), 0);
						recv(cfd, &client_pw, sizeof(client_pw), 0);
						//recv(cfd, &i, sizeof(i), 0);
			     		}
				}
				i++;
			}
		}
	}
	else if (action_num == 3)
		free(locker);
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(cfd==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(cfd);
	
	
	//i++;
	sleep(1);
}


int main()
{
	int sfd, cfd, clientlen;
	struct sockaddr_un serverUNIXaddr, clientUNIXaddr;
	pthread_t p_thread;
	
	do {
		printf("���ϴ� �繰���� ������ �Է��ϼ���(1~50) : ");
		scanf("%d", &locker_size);
	
	} while(locker_size > MAX_LOCKER_SIZE || locker_size < 1);
	locker = (struct locker *) calloc(locker_size+1, sizeof(struct locker));		

	printf("finish make locker\n");

	pthread_mutex_init(&mutx, NULL);
	//signal(SIGCHLD, SIG_IGN);

	sfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXaddr.sun_family = AF_UNIX;
	strcpy(serverUNIXaddr.sun_path, "convert");
	unlink("convert");
	bind(sfd, (struct sockaddr *)&serverUNIXaddr, sizeof(serverUNIXaddr));
	listen(sfd, 5);
	
	while(1){
		clientlen = sizeof(clientUNIXaddr);
		cfd = accept(sfd, (struct sockaddr *)&clientUNIXaddr, &clientlen);
		//printf("connect client %d\n", a);
		
		//�� �̻��� ������ ���� ��� ����
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = cfd; //���ο� ������ ������ �� clnt_cnt�� �迭 clnt_socks�� �ش� ���� ���
		pthread_mutex_unlock(&mutx);
		
		pthread_create(&p_thread, NULL, t_function, (void *)&cfd);
		pthread_detach(p_thread);
		printf("client connect: %d \n", clnt_cnt);

		if (clnt_cnt > 10)
			exit(0);		
/*
		if (thr_id < 0)
		{
			 perror("thread create error : ");
			 exit(0);
		}*/
		
		//printf("finish client %d\n", a);
		//pthread_join(p_thread, (void **)&status);

	}
   
	close(sfd);
	return 0;
}
