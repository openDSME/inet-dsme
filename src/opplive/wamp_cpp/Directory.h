#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <map>

#include "Json.h"

/**
 * Singleton
 */
class Directory
{
public:
  static Directory& getInstance();

	void insert(std::string uri, RemoteProcedure rp);
	std::pair< std::multimap<std::string,RemoteProcedure>::iterator,
	       std::multimap<std::string,RemoteProcedure>::iterator> equal_range(std::string uri);

	// TODO maybe find a better place for this
	void addConnectionHandler(std::function<void(std::string)> handler);
	void connectionEstablished(std::string client);
	
private:
	Directory();
	std::multimap<std::string, RemoteProcedure> directory;
	std::vector<std::function<void(std::string)>> connectionHandlers;
};

#endif
