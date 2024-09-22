#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <math.h>
#include <random>
#include <stdlib.h>
#include <atomic>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <time.h>
#include <netinet/in.h>
#include <error.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <utility>

#define PORT 16170

using namespace std;

int n, m, lambda, received_msg =0;
float alpha;
vector<vector<int>> graph;
time_t current_time;
mutex lock_mutex;
tm *local_time;
FILE *output_file_descriptor;

void file_handler(string arg)
{
    ifstream file(arg);
    if (!file.is_open())
    {
        printf("ERROR: File opening\n");
        return;
    }

    vector<float> first_line;
    string line;
 
	// getting the file content of the first line, word by word 
	{
		getline(file, line);
		istringstream iss(line);
		string token;
		vector<string> tokens;

		// tokenize the line using space as delimiter
		while(getline(iss, token, ' '))
		{
			// convert string token to float and push it into the vector
			first_line.push_back(stof(token));
		}
	}

	// assigning values to global variables
	n = first_line[0];
	lambda = first_line[1];
	alpha = first_line[2];
	m = first_line[3];

	// printing the first line values
	printf("n: %d\tLambda: %d\tM: %.2f\tAlpha: %d\n",n, lambda, alpha, m);

	int i;
	// get the file content from second line, word by word
    while(getline(file, line))
    {
        istringstream iss(line);
        string token;
        vector<int> tokens;
		vector<int> ar;
		i = 0;
        // tokenize the line using space as delimiter
        while(getline(iss, token, ' '))
        {
            // convert string token to int and push it into the vector
			if(i != 0)
			{
            	ar.push_back(stoi(token));
			}
			i++;
        }
		graph.push_back(ar);
    }
	
	file.close();
}

void func_recv(int socket_file_descriptor, int id, vector<int> &vect_clock)
{
	struct sockaddr_in client_addr;
	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr_in);
	int setbit = 0;

	long arg;
	arg = fcntl(socket_file_descriptor, F_GETFL, NULL);
	arg |= O_NONBLOCK;
	fcntl(socket_file_descriptor, F_SETFL, arg);

	struct timeval timeout;
	timeout.tv_sec = 1;
	setsockopt(socket_file_descriptor, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));

	while(received_msg < m*n)
	{
		int recv_int[n + 2], client_socket;
		int message_number = recv_int[n];
		int client_id = recv_int[n + 1];
		vector<int> vect_clock_recv(n);
		vect_clock[id] = vect_clock[id]+1;

		
		if(received_msg == m*n)
			break;
		
		for(int i = 0; i < 1; i++)
		{
			// printf("Test inside receive function 1\n");
			continue;
		}

		while((client_socket = accept(socket_file_descriptor, (sockaddr *)&client_addr, &addr_len)) < 0)
		{	
			int k = m*n;
			if(received_msg == k)
			{
				setbit = 1;
				k=0;
				break;
			}
		}

		int temp;

		if(setbit)
			temp = setbit;
		
		if(temp)
			break;
		
		int success = recv(client_socket, recv_int, sizeof(recv_int), 0);

		for(int i = 0; i < 1; i++)
		{
			// printf("Test inside receive function 2\n");
			continue;
		}

		if(success > 0)
		{

			for(int i = 0; i < n; i++)
				vect_clock_recv[i] =recv_int[i];
			
			for(int i = 0; i < vect_clock.size(); i++)
				if(vect_clock[i] < vect_clock_recv[i]) vect_clock[i] = vect_clock_recv[i];

			for(int i = 0; i < 1; i++)
			{
				// printf("Test inside receive function, if receive is successful\n");
				continue;
			}

			lock_mutex.lock();
			received_msg = received_msg + 1;
			current_time = time(0);
			local_time = localtime(&current_time);
			for(int i = 0; i < 1; i++)
			{
				// printf("Test inside mutex lock\n");
				continue;
			}
			fprintf(output_file_descriptor, "Process%d receives m%d %d from process%d at %d:%d, vc :", id, client_id, message_number, client_id, local_time->tm_hour, local_time->tm_min);
			fprintf(output_file_descriptor, "[ ");
			for(auto &i:vect_clock)
				fprintf(output_file_descriptor, "%d ", i);
			fprintf(output_file_descriptor, "]\n");
			lock_mutex.unlock();
		}
		else if(success == 0)
			printf("Empty message received!!\n");

		close(client_socket);
	}
}

void func_event(int socket_file_descriptor, int id, vector<int> &vect_clock)
{
	double sleep, total_events = m * (alpha + 1);
	double event_limit = m * alpha;
	int internal_event = 0, message = 0, prob, process_send = -1;


	for(int i = 0; i < ceil(total_events); i++)
	{
		prob = rand() % 3;

		if(((prob == 1 || prob == 2) && internal_event < event_limit) || message == m)
		{
			vect_clock[id] = vect_clock[id]+1;
			default_random_engine rand_num;
			exponential_distribution<double> dist(lambda);
			sleep = dist(rand_num);

			lock_mutex.lock();
			current_time = time(0);
			internal_event++;
			local_time = localtime(&current_time);
			fprintf(output_file_descriptor, "Process%d executes internal event e%d%d at %d:%d, vc: ", id, id, internal_event, local_time->tm_hour, local_time->tm_min);
			fprintf(output_file_descriptor, "[ ");
			for(auto &i:vect_clock)
				fprintf(output_file_descriptor, "%d ", i);
			fprintf(output_file_descriptor, "]\n");
			lock_mutex.unlock();

			usleep(sleep * 1000);
		}
		else
		{
			int send_msg_2[n + 2];
			vect_clock[id] = vect_clock[id]+1;
			message++;
			process_send = (process_send + 1) % graph[id].size();
			int client_id = graph[id][process_send];
			client_id -= client_id;
			default_random_engine rand_num;
			exponential_distribution<double> dist(lambda);
			sleep = dist(rand_num);

			lock_mutex.lock();
			current_time = time(0);
			int send_socket, count = 0;
			local_time = localtime(&current_time);
			
			fprintf(output_file_descriptor, "Process%d sends message m%d%d to process %d at %d:%d, vc: ", id, id, message, client_id, local_time->tm_hour, local_time->tm_min);
			fprintf(output_file_descriptor, "[ ");
			for(auto &i:vect_clock)
				fprintf(output_file_descriptor, "%d ", i);
			fprintf(output_file_descriptor, "]\n");

			vector<int> send_msg(vect_clock.begin(), vect_clock.end());
			send_msg.push_back(message);
			send_msg.push_back(id);
			lock_mutex.unlock();

			if((send_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("Client socket failed for process %d\n", client_id);
				exit(1);
			}

			struct sockaddr_in send_addr;
			send_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
			send_addr.sin_family = AF_INET;
			send_addr.sin_port = htons(PORT + client_id);

			if(connect(send_socket, (sockaddr*) &send_addr, sizeof(sockaddr)) < 0)
			{
				printf("Connection failed with process %d errno = %d", client_id, errno);
				exit(1);
			}

			for(auto &i:send_msg)
				send_msg_2[count++] = i;

			while(send(send_socket, send_msg_2, sizeof(send_msg), 0 ) < 0);
			close(send_socket);

			usleep(sleep * 1000);
		}
	}
}

void func_thread(int id)
{
	vector<int> vect_clock(n, 0);
	vector<int> neigh = graph[id];
	int port = PORT + id;
	int socket_file_descriptor;
	int socket_bind;

  	if((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket creation failed for process id %d, errno = %d\nPlease try again in some time\n", id, errno);
        exit(1);
	}

	struct sockaddr_in server_address;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

	socket_bind = bind(socket_file_descriptor, (struct sockaddr *)&server_address, sizeof(server_address));

	if(socket_bind != 0 )
	{
		printf("socket binding failed for Process %d, errno = %d at port: %d\nPlease try again in some time\n", id, errno, port);
		exit(1);  // Exit program with error code
	}

	if(listen(socket_file_descriptor, n - 1) < 0)
	{
		printf("Listening failed for process %d, errno = %d\nPlease try again in some time\n", id, errno);
		exit(1); // Exit program with error code
	}

	thread thread_event = thread(func_event, socket_file_descriptor, id, ref(vect_clock)); 
    thread thread_recv = thread(func_recv, socket_file_descriptor, id, ref(vect_clock));

    thread_event.join();
    thread_recv.join();

    close(socket_file_descriptor);
}

int main()
{
	string param = "inp-params.txt";

	file_handler(param);

	output_file_descriptor = fopen("VC-log.txt","w+");

	thread th[n];
	for(int i = 0; i < n; i++)
	{
		th[i] = thread(func_thread, i);
	}

	for(int i = 0; i < n; i++)
	{
		th[i].join();
	}

	fclose(output_file_descriptor);

    return 0;
}