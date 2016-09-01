#ifndef TOPIC_H_
#define TOPIC_H_

#include <string>
#include "Json.h"
#include <thread>
#include <mutex>

class AbstractTopic
{
public:
	virtual Json::Value getPayload() = 0;
	virtual std::string getURI() = 0;

protected:
	void publish();
};

template<typename T>
class Topic : public AbstractTopic
{
public:
	Topic(std::string uri)
	: uri(uri)
	{
	}

	void update(T val)
	{
		std::unique_lock<std::mutex> l(lock);
		payload = val;
		publish();
	}

	Json::Value getPayload()
	{
		std::unique_lock<std::mutex> l(lock);
		Json::Value val = payload; 
		return val;
	}

	std::string getURI()
	{
		return uri;
	}

private:
	std::string uri;
	T payload;
	std::mutex lock;
};

#endif
