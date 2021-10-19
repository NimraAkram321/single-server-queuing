/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, PHP, Ruby, 
C#, VB, Perl, Swift, Prolog, Javascript, Pascal, HTML, CSS, JS
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "lcgrand.h" /*header file for the random number generator*/

#define Q_LIMIT 100 //Limit on the queue lenght
#define BUSY 1 //Mnemonics for server's being BUSY
#define IDLE 0 //and IDLE

int next_event_type, num_custs_delayed, num_delays_required, num_events, num_in_q, server_status;
float area_num_in_q, area_server_status, mean_interarrival, mean_service, sim_time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3], total_of_delays;
FILE *infile, *outfile;

void initialize(void);
void timing(void);
void arrive(void);
void depart(void);
void report(void);
void update_time_avg_stats(void);
float expon(float mean);




int main(void)
{
   /*Open input and output files.*/
   infile =fopen("mm1.in","r");
   outfile= fopen("mm1.out","w");
   //specify the number of events for the timing function
   num_events=2;
   // read input parameters
   
   fscanf(infile, "%f %f %d", &mean_interarrival, &mean_service, &num_delays_required);
   //Write report heading and input parameters
   fprintf(outfile, "Single server queuing system\n\n");
   fprintf(outfile, "Mean interarrival time%11.3f minutes \n\n", mean_interarrival);
   fprintf(outfile, "Mean service time%16.3f minutes\n\n",  mean_service);
   fprintf(outfile,"Number of customers%14d\n\n", num_delays_required);
   
   //initialize the simulation
   initialize();
   
   // run the simulation while more delays are still needed
   while(num_custs_delayed< num_delays_required){
       //determine the next event
       timing();
       
       //update time_average statistical accumulators
       update_time_avg_stats();
       
       //invoke the appropriate event function
       switch(next_event_type){
        case 1:
                   arrive();
                   break;
        case 2:        
                   depart();
                   break;
       }
   }
    // invoke the report generator and end the simulation
    report();
    
    fclose(infile);
    fclose(outfile);
     
     return 0;
   
}

void initialize(void){// initialize function
      // initialize the simulation clock
      sim_time=0;
      
      // initialize the state variables
      server_status= IDLE;
      num_in_q=0;
      time_last_event=0.0;
      
      //initialize the statistical counters
      num_custs_delayed=0;
      total_of_delays=0.0;
      area_num_in_q=0.0;
      area_server_status=0.0;
      
      //initialize the event list. Since no customers are present, the departure (service completion ) event is eliminated from cosideration
      time_next_event[1]=sim_time+expon(mean_interarrival);
      time_next_event[2]=1.0e+30;
}
void timing(void){//timing function 
    int i;
    float min_time_next_event=1.0e+29;
    
    next_event_type=0;
    //determine the event type for the next event to occur
    for(i=1;i<num_events;++i){
        if(time_next_event[i]<min_time_next_event){
            min_time_next_event=time_next_event[i];
            next_event_type=i;
        }
    }
    //check to see whether the event list is empty 
    if(next_event_type==0){
        //the event list is empty, so stop the simulation 
        fprintf(outfile, "\nEvent list is empty at time %f", sim_time);
        exit(1);
    }
    //the event list is not empty so advance the simulation clock 
    sim_time=min_time_next_event;
}
void arrive(void){
    float delay;
    //schedule next arrival 
    time_next_event[1]=sim_time+expon(mean_interarrival);
    // check to see whether server is BUSY
    if(server_status==BUSY){
        //server is busy so increment the number of customers in the queue
        ++num_in_q;
        
        //Check to see whether an overflow condition exists
        if(num_in_q> Q_LIMIT){
            // the queue has overflow, so stop the simulation
            fprintf(outfile,"\nOverflowed of the array time_arrival at");
            fprintf(outfile," time %f", sim_time);
            exit(2);
        }
        // there is still room in the queue, so store the time of arrival of the arriving customer at the (new) end of the time_arrival
        time_arrival[num_in_q]=sim_time;
        
    }
    
    
    else{
        //Server is idle, so arriving customer has a delay of zero 
        delay=0.0;
        total_of_delays +=delay;
        
        //increment the number of customers delayed, and make server busy 
        ++num_custs_delayed;
        server_status=BUSY;
        
        //schedule a departure(service completion)
        time_next_event[2]=sim_time+expon(mean_service);
        
    }
}
void depart(void){
    int i;
    float delay;
    
    // checking to see whether the queue is empty
    if(num_in_q==0){
        //the queue is empty so make the server idle and eliminate the departure(service completion) event from consideration
        server_status=IDLE;
        time_next_event[2]=1.0e+30;
        
    }
    else{
        //the queue is nonempty, so decrement the number of customers in queue
        --num_in_q;
        //compute the delay of the customer who is beginning service and update the total delay of accumulator 
        delay=sim_time - time_arrival[1];
        total_of_delays += delay;
        
        //increment the number of customers delayed, and schedule departure
        ++ num_custs_delayed;
        time_next_event[2]=sim_time + expon(mean_service);
        
        //move each customer in queue (if any )up one place
        for(i=1;i<=num_in_q;++i){
            time_arrival[i]=time_arrival[i+1];
            
        } 
         
    }
}
void report(void){//report generator function
        //compute and Write estimates of desired measures of performance
        fprintf(outfile, "\nAverage delay in queue %11.3f minutes\n\n", total_of_delays/num_custs_delayed);
        fprintf(outfile, "\nAverage number in queue %10.3f \n\n", area_num_in_q/sim_time);
        fprintf(outfile, "service utilization %15.3f\n\n", area_server_status/sim_time);
        fprintf(outfile, "Time simulation ended%12.3f minutes", sim_time);
}
void update_time_avg_stats(void){// update area accumulators for time-average statistics.
        float time_since_last_event;
        
        //compute time since last event, and update last-event-time- marker.
        time_since_last_event=sim_time - time_last_event;
        time_last_event= sim_time;
        
        //Update area under number-in- queue function
        area_num_in_q += num_in_q * time_since_last_event;
        
        //Update area under server-busy indicator function
        area_server_status += server_status * time_since_last_event;
        
}
float expon(float mean){//Exponential variate generation function
        // return an Exponential random variate with mean "mean "
        return -mean * log(lcgrand(1));   
}