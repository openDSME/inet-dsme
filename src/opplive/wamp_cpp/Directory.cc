#include "Directory.h"

#include <iostream>

using namespace std;

Directory& Directory::getInstance()
{
  // instance is static - it lives forever
  static Directory instance;
  return instance;
}

Directory::Directory()
{
}

void Directory::insert(std::string uri, RemoteProcedure rp)
{
	// TODO
  directory.insert(make_pair(uri, rp));
}

std::pair< std::multimap<std::string,RemoteProcedure>::iterator,
	       std::multimap<std::string,RemoteProcedure>::iterator> Directory::equal_range(std::string uri)
{
	// TODO
	return directory.equal_range(uri);
}

void Directory::addConnectionHandler(std::function<void(std::string)> handler)
{
  connectionHandlers.push_back(handler);
}

void Directory::connectionEstablished(std::string client)
{
  for(auto handler : connectionHandlers)
  {
    if(handler)
    {
      handler(client);
    }
  }
}
