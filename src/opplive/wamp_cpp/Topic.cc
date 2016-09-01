#include "Topic.h"
#include "EventManager.h"

void AbstractTopic::publish()
{
	EventManager::getInstance().pushTopic(this);
}
