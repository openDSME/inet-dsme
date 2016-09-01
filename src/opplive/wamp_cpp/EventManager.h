#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

#include <string>
#include <map>
#include <set>
#include <functional>
#include "Json.h"
#include <queue>
#include <thread>
#include <condition_variable>
#include "Topic.h"

typedef std::function<void(std::string,std::string,Json::Value)> EventHandler;

class FunctionComparator
{
public:
  bool operator()(const std::pair<std::string,EventHandler>& a, const std::pair<std::string,EventHandler>& b) const;
};

const std::string WAMP_SERVER = "SERVER";

class EventManager
{
public:
	void subscribe(std::string client, std::string uri, EventHandler handler);
	static EventManager& getInstance();

	void publish(std::string uri, Json::Value payload, std::string exclude = "");

	template<typename T>
	void publish(std::string uri, T payload, std::string exclude = "")
	{
		publish(uri,Json::Value(payload),exclude);	
	}

	void pushTopic(AbstractTopic* topic);

private:
	EventManager();
	~EventManager();
	void eventLoop();

	bool running;
	std::thread eventThread;

	std::map<std::string, std::set<std::pair<std::string,EventHandler>,FunctionComparator>> subscriptions;
	std::mutex subscriptionsLock;

	std::queue<AbstractTopic*> pendingTopics;
	std::mutex pendingTopicsLock;
  	std::condition_variable pendingTopicsNotEmpty;
};

#endif

