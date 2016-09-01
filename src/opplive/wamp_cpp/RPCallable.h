#ifndef RPCALLABLE_H_
#define RPCALLABLE_H_

#include <functional>
#include <iostream>
#include <sstream>
#include <map>
#include <type_traits>

#include "Json.h"
#include "Directory.h"

template<int... Is>
struct seq { };

template<int N, int... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

template<int... Is>
struct gen_seq<0, Is...> : seq<Is...> { };

template< class C, typename R, class... Args, int... Is>
R callByVector(C* self, R (C::*f)(Args...), std::vector<Json::Value>& arguments, seq<Is...>)
{
	return (self->*f)(convertJson<Args>(arguments[Is])...);
}

template< class C, typename R, class... Args>
R callByVector(C* self, R (C::*f)(Args...), std::vector<Json::Value>& arguments)
{
	return callByVector(self, f, arguments, gen_seq<sizeof...(Args)>());
}

template< class C, typename R >
std::function<Json::Value(std::vector<Json::Value>)> getJsonLambda(C* self, R (C::*f)(),
		typename std::enable_if<!std::is_void<R>::value >::type* = 0)
{
	return [f,self] (std::vector<Json::Value> vals) -> Json::Value
	{ 
		return Json::Value((self->*f)());
	};
}

template< class C, typename R >
std::function<Json::Value(std::vector<Json::Value>)> getJsonLambda(C* self, R (C::*f)(), 
		typename std::enable_if<std::is_void<R>::value >::type* = 0)
{
	return [f,self] (std::vector<Json::Value> vals) -> Json::Value
	{ 
		(self->*f)();
		return Json::Value();
	};
}

template< class C, typename R, typename... Args >
std::function<Json::Value(std::vector<Json::Value>)> getJsonLambda(C* self, R (C::*f)(Args...),
		typename std::enable_if<!std::is_void<R>::value >::type* = 0)
{
	return [f,self] (std::vector<Json::Value> vals) -> Json::Value
	{ 
		return Json::Value(callByVector(self,f,vals));
	};
}

template< class C, typename R, typename... Args >
std::function<Json::Value(std::vector<Json::Value>)> getJsonLambda(C* self, R (C::*f)(Args...), 
		typename std::enable_if<std::is_void<R>::value >::type* = 0)
{
	return [f,self] (std::vector<Json::Value> vals) -> Json::Value
	{ 
		callByVector(self,f,vals);
		return Json::Value();
	};
}

template<class C>
class RPCallable
{
protected:

	template< typename R >
	void addRemoteProcedure(std::string uri, R (C::*f)()) {
		C* self = static_cast<C*>(this);
		Directory::getInstance().insert(uri, getJsonLambda(self,f));
	}

	template< typename R, typename... Args >
	void addRemoteProcedure(std::string uri, R (C::*f)(Args...)) {
		C* self = static_cast<C*>(this);
		Directory::getInstance().insert(uri, getJsonLambda(self,f));
	}

	void addConnectionHandler(std::function<void(std::string)> handler)
	{
		Directory::getInstance().addConnectionHandler(handler);
	}

private:
	std::map<std::string,std::function<Json::Value(std::vector<Json::Value>)>> callbacks;
};

#endif
