/***

Assignment5: Multilevel Feeback Queue 

Name : Anupam
Roll No : 20162118

*****/

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>

#define TIME_QUANTUM 4

using namespace std;

void RoundRobin(int, vector<vector<int> > *);

vector<vector<int> > input;	//stores processes' pid, arrival_time, burst_time, priority

vector<vector<int> > sorted_arr;
vector<vector<int> > sorted_pid;

priority_queue<pair<int, int>, vector< pair<int, int> >, greater< pair<int,int> > > pq;
queue<int> rr;	// Round Robin queue

vector<int> rem_time; 	// remaining time
vector<int> quantum(2000,0);

bool mycomp(vector<int> v1, vector<int> v2)	//sorted by arrival time
{
	if(v1[1]<v2[1])
		return true;
	return false;
}
bool mycomp2(vector<int> v1, vector<int> v2)	//sorted by pid
{
	if(v1[0]<v2[0])
		return true;
	return false;
}
int total_burstTime = 0;
int largest_arrTime = 0;

int readInput(const char *ipt)
{
	char str[20];
	int processes;
	ifstream infile(ipt);
	if(!infile.eof())
		infile.getline(str, 20);	// max of 20 characters	
	processes = atoi(str);

	for (int i = 0; i < processes; ++i)
	{
		vector<int> k;
		int pid, arrival_time, burst_time, priority;
		if(!infile.eof()) 
			infile>>pid>>arrival_time>>burst_time>>priority;
		
		k.push_back(pid);
		k.push_back(arrival_time);
		k.push_back(burst_time);
		k.push_back(priority);

		total_burstTime += burst_time;
		if(largest_arrTime < arrival_time)
			largest_arrTime = arrival_time;

		input.push_back(k);
	}
	infile.close();
	sorted_arr = input;
	sorted_pid = input;
	sort(sorted_arr.begin(), sorted_arr.end(), mycomp);
	sort(sorted_pid.begin(), sorted_pid.end(), mycomp2);
	return processes;
}

/**** start of main ***/
int main(int argc, char const *argv[])
{
	int processes;
	processes = readInput(argv[1]);	//input file

	vector<bool> finishedPQ(processes, false);	//true if processes finished in Priority Queue

	vector<vector<int> > output;	//responseTime, finishing time, waiting time
	for (int i = 0; i < processes; ++i)
	{
		vector<int> v1(3,-1);	// 3 elements each initialized to -1
		output.push_back(v1);
	}
	for (vector<vector<int> >::iterator i = sorted_pid.begin(); i != sorted_pid.end(); ++i)
	{
		vector<int> vtr = *i;
		rem_time.push_back(vtr[2]);
	}
	int index_at = 0;
	int inCPU;		//holds pid presently in CPU
	int last_inCPU=-1;	//holds pid of process which held CPU in last tick
	for( int t=0; t < largest_arrTime+total_burstTime; t++)
	{
		//while - for processes which arrived at same arrival time
		while(index_at < sorted_arr.size() && sorted_arr[index_at][1] == t)
		{	
				//sorted_arr[index_at][3] is priority and sorted_arr[index_at][0]  is pid of process at index_at
				pq.push(make_pair(sorted_arr[index_at][3], sorted_arr[index_at][0]));
				index_at++;		
		}
		if(!pq.empty())
		{
			pair<int, int> pr = pq.top();			
			//	first is priority  and second is pid
			inCPU = pr.second;
			// to care of processes finished in PQ but not popped
			if(finishedPQ[inCPU-1] == true)
			{
				pq.pop();
				t--;	//becoz it wasted 1 sec
				continue;
			}
			//handles when new process arrives when CPU is executing RR Queue
			if(inCPU != -1 && !rr.empty() && last_inCPU != -1)
			{
				int frnt = rr.front();
				if(quantum[frnt-1]>0 && quantum[frnt-1]<=4)
				{
					rr.pop();
					quantum[frnt-1] = 0;
					rr.push(frnt);
				}
			}
			if(inCPU != last_inCPU && last_inCPU != -1 && !finishedPQ[last_inCPU-1])
			{
				// take care of preempting a process even when the quanta is not finished
				// bcoz of arrival of new high priority process 
				rr.push(last_inCPU);
				finishedPQ[last_inCPU-1] = true;
			}
			//handling current process in priority queue
			if(output[inCPU-1][0] == -1)
				output[inCPU-1][0] = t - sorted_pid[inCPU-1][1];	// response time = cur_time - arr_time
			rem_time[inCPU-1]--;
			if (rem_time[inCPU-1] == 0)
			{
				output[inCPU-1][1] = t+1;	//finishing_time
				output[inCPU-1][2] = output[inCPU-1][1] - sorted_pid[inCPU-1][1] - sorted_pid[inCPU-1][2];	//waiting_time
				pq.pop();
				finishedPQ[inCPU-1] = true;
			}
			else if((sorted_pid[inCPU-1][2] - rem_time[inCPU-1]) == TIME_QUANTUM)	
			{
				// burstTime - RemTime == TIME_QUANTUM
				rr.push(inCPU);
				pq.pop();
				finishedPQ[inCPU-1] = true;
			}
			last_inCPU = inCPU;
		}
		else if(!rr.empty())
		{
			last_inCPU = -1;
			inCPU = -1;
			RoundRobin(t, &output);
		}
		
	}
	//outputs to file given in argv[2]
	int pid=1;
	ofstream ofile(argv[2]);
	for (vector<vector<int> >::iterator i = output.begin(); i != output.end(); ++i)
	{
		vector<int> vtr = *i;
		ofile<<pid++<<" "<<vtr[0]<<" "<<vtr[1]<<" "<<vtr[2]<<endl;
	}
	ofile.close();
	return 0;
}
// handles RoundRobin each time
void RoundRobin(int time, vector<vector<int> > *output)
{
	int pid = rr.front();

	quantum[pid-1]++;
	rem_time[pid-1]--;
	if(rem_time[pid-1] == 0)
	{
		(*output)[pid-1][1] = time+1;	//finishing_time
		(*output)[pid-1][2] = (*output)[pid-1][1] - sorted_pid[pid-1][1] - sorted_pid[pid-1][2];	//waiting_time
		rr.pop();
		return;
	}
	if(quantum[pid-1]==4 && rem_time[pid-1] != 0)
	{
		quantum[pid-1]=0;
		rr.push(rr.front());
		rr.pop();
	}
}
