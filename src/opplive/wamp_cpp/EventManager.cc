#include "EventManager.h"
#include "Directory.h"

using namespace std;

EventManager::EventManager()
{
	running = true;
	eventThread = std::thread(&EventManager::eventLoop,this);
}

EventManager::~EventManager()
{
	running = false;
	eventThread.join();
}

bool FunctionComparator::operator()(const pair<string,EventHandler>& a, const pair<string,EventHandler>& b) const
{
  auto aptr = a.second.target<void(string,Json::Value)>();
  auto bptr = b.second.target<void(string,Json::Value)>();

  return (aptr < bptr) || (aptr == bptr && a.first < b.first);
}

void EventManager::subscribe(string client, string uri, EventHandler handler)
{
    	unique_lock<mutex> l(subscriptionsLock);
	subscriptions[uri].insert(make_pair(client,handler));
}

EventManager& EventManager::getInstance()
{
	static EventManager eventManager;
	return eventManager;
}
	
void EventManager::publish(std::string uri, Json::Value payload, std::string excluded)
{
	// create values vector
	vector<Json::Value> values;
	if(payload.isArray())
	{
		values.resize(payload.size());
		for(unsigned int i = 0; i < payload.size(); i++)
		{
			values[i] = payload[i];
		}
	}
	else
	{
		values.push_back(payload);
	}

	// execute associated rpcs
	if(excluded != WAMP_SERVER)
	{
		auto range = Directory::getInstance().equal_range(uri);
		for(auto it = range.first; it != range.second; it++)
		{
			RemoteProcedure& rp = it->second;
			if(rp)
			{
				// TODO what if values does not match??
				// TODO future!!
				rp(values);
			}
		}
	}

	unique_lock<mutex> l(subscriptionsLock);
	auto ehSet = subscriptions.find(uri);

	// has someone subscribed to this event?
	if(ehSet != subscriptions.end())
	{
		for(auto eh : (*ehSet).second)
		{
			if(eh.first != excluded)
			{
				eh.second(eh.first,uri,payload);
			}
		}
	}
}

void EventManager::pushTopic(AbstractTopic* topic)
{
    	unique_lock<mutex> l(pendingTopicsLock);
	pendingTopics.push(topic);
	pendingTopicsNotEmpty.notify_one();
}

void EventManager::eventLoop()
{
    	unique_lock<mutex> l(pendingTopicsLock);

	while(running)
	{
		while(!pendingTopics.empty())
		{
			AbstractTopic* topic = pendingTopics.front();
			pendingTopics.pop();
			l.unlock();

			string uri = topic->getURI();
			Json::Value payload = topic->getPayload();
			publish(uri,payload,WAMP_SERVER);
			l.lock();
		}

    		pendingTopicsNotEmpty.wait_for(l, chrono::milliseconds(500));
	}
}

