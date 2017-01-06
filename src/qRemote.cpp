#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int32.h"
#include "std_msgs/Bool.h"
#include <sstream>
#include <iostream>
#include <string.h>
#include <signal.h>
#include <thread>
#include <quadMsgs/qParameters.h>
#include <quadMsgs/qStatus.h>

#define QUAD_ID 0x1289

using namespace std;

static char cIn = 0;
static int32_t number = 0;
static bool Arm = false;
static int32_t throttle = 0,p = 0,d = 0,i=0,pa=0,ta=0;

void checkCin()
{
	while(1)
	{
		std::cin>>cIn;
		if(cIn=='n')
		{
			std::cin>>number;
			cIn=0;
		}
	}
}

void updateArm(ros::Publisher* rosP)
{
	std_msgs::Bool value;
	value.data = Arm;
	rosP->publish(value);
	ros::spinOnce();
}

void updateValues(uint32_t qid,ros::Publisher* rosP, int32_t* th, int32_t* pv, int32_t* iv, int32_t* dv, int32_t* pav, int32_t* tav)
{
	quadMsgs::qParameters value;
	value.qID = qid;
	value.qThrottle = *th;
	value.qP = *pv;
	value.qI = *iv;
	value.qD = *dv;
	value.qPA = *pav;
	value.qTargetAngle = *tav;
	rosP->publish(value);
	ros::spinOnce();
	cout<<"T,P,I,D,PA,TA = "<<*(th)<<','<<*(pv)<<','<<*(iv)<<','<<*(dv)<<','<<*(pav)<<','<<*(tav)<<"\n";
}

void showStatus()
{
	static int dots = 0;
	dots++;
	if(Arm==true) std::cout<<((dots<15)?".":"\n");
	if(dots>=15) dots=0;
}

int main(int argc, char **argv)
{
	uint32_t count = 0;
	char currentVariable = 'p';
	ros::init(argc, argv, "quadRemote");
	ros::NodeHandle n;
	ros::Publisher quadArm = n.advertise<std_msgs::Bool>("quadArm", 10);
	ros::Publisher quadParam = n.advertise<quadMsgs::qParameters>("quadParam", 10);
	ros::Rate loop_rate(20);
	std::thread getC(checkCin);
	std::cout<<"UnArmed\n";
	while(ros::ok())
	{
		if(count>20)
		{
			updateArm(&quadArm);
			showStatus();
			count=0;
		}
		count++;
		if(cIn!=0)
		{
			switch(cIn)
			{
				case 'q':
					Arm = !Arm;
					if(Arm == false)
					{
						throttle = 0;
						p=0;
						d=0;
						i=0;
						pa=0;
						ta=0;
					}
					std::cout<<(Arm?"Armed\n":"UnArmed\n");
					break;
				case 'z':
					throttle = 0;
					p=0;
					d=0;
					i=0;
					pa=0;
					ta=0;
					break;
				case 'w':
					throttle++;
					break;
				case 's':
					throttle--;
					break;
				case 't':
					if(number!=0) throttle=number;
					number=0;
					break;
				case 'p':
					if(number!=0) p=number;
					else currentVariable='p';
					number=0;
					break;
				case 'i':
					if(number!=0) i=number;
					else currentVariable='i';
					number=0;
					break;
				case 'd':
					if(number!=0) d=number;
					else currentVariable='d';
					number=0;
					break;
				case 'a':
					if(number!=0) pa=number;
					else currentVariable='a';
					number=0;
					break;
				case 'l':
					if(number!=0) ta=number;
					else currentVariable='l';
					number=0;
					break;
				case ']':
					switch(currentVariable)
					{
						case 'p':
							p+=10;
							break;
						case 'd':
							d+=1;
							break;
						case 'i':
							i+=1;
							break;
						case 'a':
							pa+=1;
							break;
						case 'l':
							ta+=1;
							break;
					}
					break;
				case '[':
					switch(currentVariable)
					{
						case 'p':
							p-=10;
							break;
						case 'd':
							d-=1;
							break;
						case 'i':
							i-=1;
							break;
						case 'a':
							pa-=1;
							break;
						case 'l':
							ta-=1;
							break;
					}
					break;
				default:
					std::cout<<"Not valid input\n";
					break;
			}
			updateValues(QUAD_ID,&quadParam,&throttle,&p,&i,&d,&pa,&ta);
			cIn = 0;
		}
		loop_rate.sleep();
	}
	Arm = false;
	updateArm(&quadArm);
	getC.detach();
}
