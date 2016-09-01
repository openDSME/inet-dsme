#include <WAMPMessageHandler.h>
#include "Directory.h"
#include "EventManager.h"

using namespace std;
using namespace std::placeholders;

void WAMPMessageHandler::sendCallResult(string client, string callID, Json::Value result)
{
	Json::FastWriter writer;
	stringstream output;
	output << "[" << CALLRESULT << ",\"" << callID << "\"," << writer.write(result) << "]";

	if(sendFun)
	{
	  sendFun(client,output.str());
	}
}



void WAMPMessageHandler::handleCall(string client, string callID, string uri, vector<Json::Value> values)
{
// TODO is this against the standard? Probably!
	auto range = Directory::getInstance().equal_range(uri);
	for(auto it = range.first; it != range.second; it++)
	{
		Json::Value result;

		RemoteProcedure& rp = it->second;

		if(rp)
		{
			// TODO what if values does not match??
			// TODO future!!
			result = rp(values);
			sendCallResult(client,callID,result);
		}
		else
		{
			// send error!
		}
	}
}

void WAMPMessageHandler::sendEvent(string client, string uri, Json::Value payload)
{
	Json::FastWriter writer;
	stringstream output;
	output << "[" << EVENT << ",\"" << uri << "\"," << writer.write(payload) << "]";

	if(sendFun)
	{
	  sendFun(client,output.str());
	}
}

void WAMPMessageHandler::subscribe(string client, string uri)
{
  auto eventHandler = bind(&WAMPMessageHandler::sendEvent,this,_1,_2,_3); 
  //EventManager::getInstance().subscribe(uri,static_cast<EventHandler>(eventHandler));
  EventManager::getInstance().subscribe(client,uri,eventHandler);
}

void WAMPMessageHandler::receiveMessage(std::string client, std::string msg)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	stringstream input;
	input << msg;
	bool parsingSuccessful = reader.parse( msg, root );
	if ( !parsingSuccessful )
	{
		std::cout  << "Failed to parse configuration\n"
			<< reader.getFormatedErrorMessages();
		return;
	}

	if(root.size() < 1 || !root[(unsigned int)0].isInt())
	{
		cerr << "Wrong message format" << endl;
	}
	
	vector<Json::Value> values;

	try
	{
		switch(root[(unsigned int)0].asInt())
		{
		case CALL:
			if(root.size() < 3)
			{
			  cerr << "Too few elements in message for CALL" << endl;
			}

			for(unsigned int i = 3; i < root.size(); i++)
			{
				values.push_back(root[i]);
			}

			handleCall(client,root[1].asString(),root[2].asString(),values);
			break;

		case SUBSCRIBE:
			if(root.size() < 2)
			{
			  cerr << "Too few elements in message for SUBSCRIBE" << endl;
			}
			
			subscribe(client, root[1].asString());
			break;

		case PUBLISH:
			switch(root.size())
			{
			case 3:
				EventManager::getInstance().publish(root[1].asString(),root[2],"");
				break;
			case 4:
			case 5:
				cout << root[1].asString() << " " << root[3].asBool() << endl;
				EventManager::getInstance().publish(root[1].asString(),root[2],root[3].asBool()?client:"");
				break;
			default:
				cerr << "Wrong format for PUBLISH" << endl;
				
			};
			break;

		default:
			cerr << "Message type " << root[(unsigned int)0].asInt() << "  not implemented"  << endl;
		}
	}
	catch(const runtime_error& e)
	{
		cerr << "Message '" << msg << "' could not be processed: " << e.what() << endl;
	}
}

void WAMPMessageHandler::registerSend(function<void(std::string,std::string)> send)
{
  sendFun = send; 
}

