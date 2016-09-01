#include "DemoServer.h"
#include "EventManager.h"
#include <iostream>

using namespace std;

DemoServer::DemoServer()
: topic("http://example.com/simple/ev3")
{
  addRemoteProcedure("http://example.com/simple/calc#add",&DemoServer::adding);
  addRemoteProcedure("http://example.com/simple/ev1",&DemoServer::handleEvent1);

  running = true;
  eventThread = thread(&DemoServer::eventLoop,this);
}

DemoServer::~DemoServer()
{
  running = false;
  std::cout << "DemoServer shutting down" << std::endl;
  eventThread.join();
  std::cout << "DemoServer shut down" << std::endl;
}

int DemoServer::adding(int a, int b)
{
  return a+b;
}

void DemoServer::handleEvent1(int a)
{
  std::cout << "Event 1 received " << a << std::endl;
}

void DemoServer::eventLoop()
{
	while(running)
	{
		this_thread::sleep_for(chrono::seconds(5));

		/* Use EventManager directly */
		EventManager::getInstance().publish("http://example.com/simple/ev2", 55);

		/* Publish to predefined topic */
		topic.update(12);
	}
}

