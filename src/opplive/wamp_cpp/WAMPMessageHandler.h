#ifndef WAMPMESSAGEHANDLER_H_
#define WAMPMESSAGEHANDLER_H_
#include <iostream>
#include <sstream>
#include <functional>
#include <memory>

#include "Json.h"

class WAMPMessageHandler
{
public:
  enum WAMPMessage
  {
	WELCOME,
	PREFIX,
	CALL,
	CALLRESULT,
	CALLERROR,
	SUBSCRIBE,
	UNSUBSCRIBE,
	PUBLISH,
	EVENT
  };

  void sendCallResult(std::string client, std::string callID, Json::Value result);
  void handleCall(std::string client, std::string callID, std::string uri, std::vector<Json::Value> values);
  void sendEvent(std::string client, std::string uri, Json::Value payload);
  void subscribe(std::string client, std::string uri);
  void receiveMessage(std::string client, std::string msg);
  void registerSend(std::function<void(std::string,std::string)> send);

  std::function<void(std::string,std::string)> sendFun;
};
#endif
